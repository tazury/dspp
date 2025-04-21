#include <Windows.h>
#include <DualSenseMgr.h>
#include <Logger.h>
#include <iostream>
#include <atomic>
#include <thread>

std::atomic<bool> running = true;
DualSenseMgr* mgr = nullptr;

// Windows API function to handle console close event
BOOL WINAPI ConsoleHandler(DWORD signal) {
    if (signal == CTRL_CLOSE_EVENT || signal == CTRL_C_EVENT) {
        std::cout << "Console closing, resetting controller...\n";

        running = false;  // Stop the loop

        // Wait a bit to let main handle cleanup properly
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));

        return TRUE;
    }
    return FALSE;
}

int main() {
    // Register the console close handler
    SetConsoleCtrlHandler(ConsoleHandler, TRUE);

    // Logger setup
    DSLogger::addListener([](const LogEntry& entry) {
        const char* typeStr = "Unknown";
        switch (entry.type) {
        case DSLogTypes::Normal: typeStr = "Normal"; break;
        case DSLogTypes::Warning: typeStr = "Warning"; break;
        case DSLogTypes::Error: typeStr = "Error"; break;
        }
        std::cout << "[" << typeStr << "] "
            << entry.file << ":" << entry.line << " - "
            << entry.message << "\n";
        });

    // Controller setup
    DualSenseMgr localMgr;
    mgr = &localMgr;  // Assign global pointer

    DualSenseMgr::DiscoverSettings set;
    set.maxPlayers = 8;

    DualSense::DualSense* ctrl = nullptr;
    bool listenerSetup = false;
    bool waitForRec = false;


    mgr->discoverInBackground(set, [&](DualSenseMgr::CtrlIncomingInfo info) {

        if (info.reconnection) {
            listenerSetup = false;
            ctrl = nullptr;
            waitForRec = true;
        }

        return true;
        
        });

    while (running) {
       
        //if (waitForRec) {
        //    std::this_thread::sleep_for(std::chrono::milliseconds(50));
        //    waitForRec = false;
        //}
        

        if (ctrl && ctrl->controller.is_connected) {
            if (!listenerSetup) {
                std::thread([&ctrl]() {

                    ctrl->input.connection = ctrl->controller.is_usb; // ill make it automatic
                    ctrl->input.addListener([&ctrl](DualSense::Input::InputState state, double timestamp) {
                        std::wstring title = L"DSPP Input Listener Player 1";
                        SetThreadDescription(GetCurrentThread(), title.c_str());
                        static bool wasMutedPressed = false;

                        bool isAnyB = false;
                        for (bool button : state.buttons) {
                            if (button) isAnyB = true;
                        }

                        if (isAnyB) {
                            ctrl->output.leftMoter = 100;
                            ctrl->output.rightMoter = 100;
                            ctrl->output.overallMotors = 100;
                        }
                        else {
                            ctrl->output.leftMoter = 0;
                            ctrl->output.rightMoter = 0;
                            ctrl->output.overallMotors = 0;
                        }

                        bool isMutedPressed = state.buttons[static_cast<size_t>(DualSense::Input::Button::Mute)];

                        bool isCross = state.buttons[static_cast<size_t>(DualSense::Input::Button::Cross)];

                        if (isCross) {
                            std::cout << "cross pressed : " << timestamp << "\n";
                        }

                        // OnRelease: Detects when the mute button is released
                        if (!isMutedPressed && wasMutedPressed) {
                            ctrl->input.isMuted = !ctrl->input.isMuted;
                            ctrl->output.ledSettings.muteLED = ctrl->input.isMuted;
                        }

                        wasMutedPressed = isMutedPressed; // Update state

                        float leftTrigger = state.axes[static_cast<size_t>(DualSense::Input::Axis::L2Trigger)];
                        if (leftTrigger > 0.5f) {
                            std::cout << "Left trigger halfway pressed: " << leftTrigger << std::endl;
                        }


                        float leftStickX = state.axes[static_cast<size_t>(DualSense::Input::Axis::LeftStickY)];
                        if (leftStickX > 0.5f) {
                            std::cout << "Left stick halfway pressed: " << leftStickX << std::endl;
                        }


                        // Testing out the accelerometer!
                        const float THRESHOLD_X = 0.5f; // Tilt threshold for "NO" (siding)
                        const float THRESHOLD_Y = 0.5f; // Tilt threshold for "YES" (nodding)

                        float accX = state.axes[static_cast<size_t>(DualSense::Input::Axis::AccelerometerX)];
                        float accY = state.axes[static_cast<size_t>(DualSense::Input::Axis::AccelerometerY)];
                        // float accZ = state.axes[static_cast<size_t>(Dualsense::Input::Axis::AccelerometerZ)];


                         //YES
                        if (accY > THRESHOLD_Y) {
                            std::cout << "You're saying yes!" << std::endl;
                        }
                        // NO
                        else if (std::abs(accX) > THRESHOLD_X) {
                            std::cout << "You're saying no :(" << std::endl;
                        }

                        float gyroX = state.axes[static_cast<size_t>(DualSense::Input::Axis::GyroscopeX)];
                        /* float gyroY = state.axes[static_cast<size_t>(Dualsense::Input::Axis::GyroscopeY)];
                         float gyroZ = state.axes[static_cast<size_t>(Dualsense::Input::Axis::GyroscopeZ)];*/

                        const float ROTATION_THRESHOLD = 15.5f;  // Adjust as needed for sensitivity

                        // BARREL ROLL!!! - 100% sure this isn't accurate but this seems good enough for a test
                        if (std::abs(gyroX) > ROTATION_THRESHOLD) {
                            std::cout << "BARREL ROLLLLLLLLL" << std::endl;
                        }

                        if (state.touchPoints[0].active) {
                            std::cout << "Touchpad 0 X:" << state.touchPoints[0].x << std::endl;
                            std::cout << "Touchpad 0 Y:" << state.touchPoints[0].y << std::endl;

                        }
                        if (state.touchPoints[1].active) {
                            std::cout << "Touchpad 1 X:" << state.touchPoints[1].x << std::endl;
                            std::cout << "Touchpad 1 Y:" << state.touchPoints[1].y << std::endl;
                        }
                        });


                    std::cout << "Starting PlayStation Input Thread...\n";
                    ctrl->input.start();
                    }).detach();
                listenerSetup = true;
            }
          //  ctrl->output.leftMoter = 25;
            ctrl->output.sendOutput();
        }
        else {

            ctrl = mgr->getController(1);
        }

      std::this_thread::sleep_for(std::chrono::milliseconds(100)); // Reduce CPU usage
    }

    std::cout << "Cleaning up resources...\n";
    return 0;  // Destructor of `localMgr` will be called automatically here
}
