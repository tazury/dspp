#include "DualSenseMgr.h"
#include "Logger.h"
#include <hidapi/include/hidapi.h>
#include <iostream>
#include "utils.h"

#define VENDOR_SONY    0x054C
#define PRODUCT_DUALSENSE 0x0CE6



DualSenseMgr::~DualSenseMgr()
{
    justStop = true;
    for (const auto& [id, controller] : controllers) { 
        controller->output.Reset();
        controller->input.stop();
        hid_close(controller->controller.hid_dev);
    }
    if (discoveryThread.joinable()) {
        discoveryThread.join();
    }
    for (auto& thread : cThreads) {
        thread.join();
    }
    hid_exit();
}

void DualSenseMgr::discoverInBackground(DiscoverSettings settings)
{
    DS_BASEDIR(DSBASEDIR);
    // Create the thread!
    discoveryThread = std::thread([&]() { discoverHelper(settings, false); });
}

void DualSenseMgr::discoverInBackground(DiscoverSettings settings, DiscoveryListener listener)
{
    DS_BASEDIR(DSBASEDIR);
    // Add the listener
    std::lock_guard<std::mutex> lock(dlistenersMutex);
    dlistener = std::move(listener);
    // Create the thread!
    discoveryThread = std::thread([&]() { discoverHelper(settings, true); });
}

bool DualSenseMgr::discoverNow(DiscoverSettings settings)
{
    DS_BASEDIR(DSBASEDIR);
	return discoverHelper(settings, false);
}

bool DualSenseMgr::notifyDListeners(CtrlIncomingInfo info)
{
    std::lock_guard<std::mutex> lock(dlistenersMutex);
    return dlistener(info);
}

std::map<int, DualSense::DualSense*> DualSenseMgr::getControllers()
{
    return controllers;
}

DualSense::DualSense* DualSenseMgr::getController(int id)
{
    if(id == 0){
        DS_LOG_WARNING("Getting controller from ID 0 is inproper, please read the documentation. Changing to 1");
        id = 1;
    }

    auto it = controllers.find(id-1);
    if (it != controllers.end()) {
        return it->second;
    }
    else return nullptr;
}

int DualSenseMgr::getAmoOfPlayers()
{
    return static_cast<int>(controllers.size());
}

bool DualSenseMgr::discoverHelper(DualSenseMgr::DiscoverSettings settings, bool isThreaded)
{
    std::string title = "DSPP Discovery Helper";
    SetCurrentThreadName(title);
        long long sTime = getCurrentTimestamp();
        int pDetected = 0;
        while (pDetected < settings.maxPlayers && !justStop || pDetected < settings.minPlayers && !justStop) {
            hid_device_info* devs = hid_enumerate(0, 0);
            if (settings.timeout > 1 && (getCurrentTimestamp() - sTime) >= settings.timeout) break; // TIMEOUT
            for (hid_device_info* cur = devs; cur; cur = cur->next) {
                // Detect PlayStation
                if (cur->vendor_id == VENDOR_SONY && cur->product_id == PRODUCT_DUALSENSE) {
                    std::string devicePath = cur->path;

                    // If we've already seen this device path, skip it
                    if (seenDevicePaths.find(devicePath) != seenDevicePaths.end()) {
                        continue;
                    }



                    int playerId = 1;
                    bool isAlrReplaced = false; // Is already replaced
                    for (const auto& [id, controller] : controllers) {
                        if (!controller->controller.is_connected && !isAlrReplaced) {
                            isAlrReplaced = true;
                            playerId = controller->controller.number;
                        }
                    }

                    if (!isAlrReplaced) {
                        playerId = getAmoOfPlayers() + 1; // Assuming new controller
                    }

                    DS_LOG_NORMAL("Controller found : Player/ID" + std::to_string(playerId));
                    DualSense::DualSense* c = new DualSense::DualSense();
                    c->controller.is_connected = true;
                    c->controller.hid_dev = hid_open_path(cur->path);
                    std::wcout << hid_error(c->controller.hid_dev) << std::endl;
                    c->controller.is_usb = cur->bus_type != 2;
                    c->controller.number = playerId;

                    if (isThreaded) {
                        DualSenseMgr::CtrlIncomingInfo info;
                        info.playerNumber = c->controller.number;
                        if (isAlrReplaced) info.reconnection = true;
                        info.timestamp = getCurrentTimestamp();
                        info.cInfo = &c->controller;
                        if (notifyDListeners(info)) {
                            if (isAlrReplaced) {
                              
                            }
                            DS_LOG_NORMAL("Controller accepted : Player/ID" + std::to_string(c->controller.number));
                            c->output.ledSettings.playerNumber = c->output.getLEDNumber();
                            controllers[c->controller.number-1] = c;
                            cThreads.push_back(std::thread([&]() { connectionThread(controllers[c->controller.number-1]); }));
                            if (!isAlrReplaced) pDetected++;
                        }
                        else {
                            DS_LOG_WARNING("Controller denied : Player/ID" + std::to_string(c->controller.number));
                        }
                    }
                    else { // NOT THREADED
                      //  if (isAlrReplaced) controllers.erase(c->controller.number); // A controller already.
                        DS_LOG_NORMAL("Controller accepted by default : Player/ID " + std::to_string(c->controller.number));
                        c->output.ledSettings.playerNumber = c->output.getLEDNumber();
                        controllers[c->controller.number-1] = c;
                        cThreads.push_back(std::thread([&]() { connectionThread(controllers[c->controller.number-1]); }));
                        if (!isAlrReplaced) pDetected++;
                    }
                    seenDevicePaths.insert(devicePath);
                }

            }
            hid_free_enumeration(devs);
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }

        return true;

}

// If the connection is -1 for 3 times, thats not stable
void DualSenseMgr::connectionThread(DualSense::DualSense* ds)
{
    std::string title = "DSPP Connection Thread Player " + std::to_string(ds->controller.number);
    SetCurrentThreadName(title);
    if(ds->controller.hid_dev == nullptr){
        DS_LOG_WARNING("Controller disconnected : Player/ID " + std::to_string(ds->controller.number));
        ds->controller.is_connected = false;
        return;
    }
    hid_set_nonblocking(ds->controller.hid_dev, 1);
    int reportSize = 0;
    std::vector<uint8_t> buffer;

            int bytesR = 0;
            int amFail = 0;
    while (ds->controller.is_connected && !justStop) {
        if (ds->controller.is_usb) {
            reportSize = 77;
            buffer = std::vector<uint8_t>(reportSize);
            buffer[0] = 0x31;
        }
        else {
            reportSize = 78;
            buffer = std::vector<uint8_t>(reportSize);
            buffer[0] = 0x31;
        }
        ds->controller.is_checking_connection = true;

        bytesR = hid_read(ds->controller.hid_dev, buffer.data(), reportSize);
        if (bytesR < 0) {
            amFail++;
        }
        else {
            amFail = 0;
        }


        if (amFail == 3) {
            ds->controller.is_checking_connection = false;
            DS_LOG_WARNING("Controller disconnected : Player/ID " + std::to_string(ds->controller.number));
            ds->controller.is_connected = false; // Controller is Discomnected!
            seenDevicePaths.erase(hid_get_device_info(ds->controller.hid_dev)->path);
            controllers.erase(ds->controller.number-1); // A controller already.
        }
        
        if (amFail == 0) {
            ds->controller.is_checking_connection = false;

            std::this_thread::sleep_for(std::chrono::milliseconds(2));
        }
    }
}
