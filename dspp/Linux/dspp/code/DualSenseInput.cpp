#include "DualSenseInput.h"
#include "DualSense.h"
#include "Logger.h"
#include <pthread.h>
#include <string>
#include "utils.h"

namespace DualSense {

    Input::~Input() {
        stop();
    }

	void Input::addListener(Listener listener) {
        std::lock_guard<std::mutex> lock(listenersMutex);
        listeners.push_back(std::move(listener));
    }

    bool Input::start() {
      
        this->device = controller.hid_dev;
        this->connection = controller.is_usb;


        if (running.load()) return false;
         


        running.store(true);


        if (!controller.is_usb) {
            workerThread = std::thread([this]() {
                std::wstring title = L"DualSense Input Worker Player " + std::to_wstring(controller.number);
                DS_LOG_NORMAL("Input Thread has stared for PID" + std::to_string(controller.number));
                const int reportSize = 78;
                std::vector<uint8_t> buffer(reportSize);
                buffer[0] = 0x31;

        
                while (this->running.load()) {
                    while (controller.is_checking_connection) {
                        // waiting
                    }
                    if (!controller.is_connected) return;
                    int bytesRead = 0;
                    try {
                        bytesRead = hid_read(controller.hid_dev, buffer.data(), reportSize);
                    }
                    catch (const std::exception& ex) {  // Catch by reference
                        DS_LOG_ERROR(std::string("EXCEPTION OCCURRED ON GETTING INPUT: ") + ex.what());
                    }
                    if (bytesRead > 0) {
                        InputState state = parseReport(buffer);
                        double timestamp = getCurrentTimestamp();
                        notifyListeners(state, timestamp);
                    }
                    std::this_thread::sleep_for(std::chrono::milliseconds(1));
                }
                });
        }
        else {

            workerThread = std::thread([this]() {
                std::string title = "DSPP Input Worker Player " + std::to_string(controller.number);
                SetCurrentThreadName(title);
                DS_LOG_NORMAL("Input Thread has stared for PID" + std::to_string(controller.number));
                const int reportSize = 77;
                std::vector<uint8_t> buffer(reportSize);
                buffer[0] = 0x31;

                

                while (this->running.load()) {
                    while (controller.is_checking_connection) {
                        // waiting
                    }
                    if (!controller.is_connected) return;
                    int bytesRead = 0;
                    try {
                         bytesRead = hid_read(controller.hid_dev, buffer.data(), reportSize);
                    }
                    catch (const std::exception& ex) {  // Catch by reference
                        DS_LOG_ERROR(std::string("EXCEPTION OCCURRED ON GETTING INPUT: ") + ex.what());
                    }
                    if (bytesRead > 0) {
                        InputState state = parseReport(buffer);
                        double timestamp = getCurrentTimestamp();
                        notifyListeners(state, timestamp);
                    }
                    std::this_thread::sleep_for(std::chrono::milliseconds(1));
                }
                });
        }

        return true;
    }
    void Input::stop()
    {
        running = false;
        if (workerThread.joinable()) {
            workerThread.join();
        }
    }

    void Input::notifyListeners(const InputState& state, double timestamp)
    {
        std::lock_guard<std::mutex> lock(listenersMutex);
        for (auto& listener : listeners) {
            listener(state, timestamp);
        }
        
    }

    Input::InputState Input::parseReport(const std::vector<uint8_t>& report)
    {

        InputState state{};

        // Digital buttons (common for USB and Bluetooth)
        parseButtons(report, state, connection, report[0]);

        // Analog sticks and triggers
        parseAnalogInputs(report, state, connection, report[0]);

        // Touchpad
        parseTouchpad(report, state, connection, report[0]);

        // Motion controls
        parseMotion(report, state, connection, report[0]);

        // Microphone status
        state.microphoneMuted = (report[47] & 0x01) != 0;

        return state;
    }

    void Input::parseButtons(const std::vector<uint8_t>& report, InputState& state, bool connectionType, int reportID)
    {
        if (report.size() < 11) return; // Ensure report has enough data

        if (reportID == 0x01 && connectionType) {

            // The button bit field (3 bytes)
            uint32_t buttons = (report[8] << 0) | (report[9] << 8) | (report[10] << 16);

            // Map buttons based on known bit positions
            state.buttons[static_cast<size_t>(Button::Cross)] = (buttons & 0x20) != 0;
            state.buttons[static_cast<size_t>(Button::Circle)] = (buttons & 0x40) != 0;
            state.buttons[static_cast<size_t>(Button::Square)] = (buttons & 0x10) != 0;
            state.buttons[static_cast<size_t>(Button::Triangle)] = (buttons & 0x80) != 0;

            state.buttons[static_cast<size_t>(Button::L1)] = (buttons & 0x100) != 0;
            state.buttons[static_cast<size_t>(Button::R1)] = (buttons & 0x200) != 0;
            state.buttons[static_cast<size_t>(Button::L2)] = (buttons & 0x400) != 0;
            state.buttons[static_cast<size_t>(Button::R2)] = (buttons & 0x800) != 0;

            state.buttons[static_cast<size_t>(Button::Share)] = (buttons & 0x1000) != 0;
            state.buttons[static_cast<size_t>(Button::Options)] = (buttons & 0x2000) != 0;

            state.buttons[static_cast<size_t>(Button::L3)] = (buttons & 0x4000) != 0;
            state.buttons[static_cast<size_t>(Button::R3)] = (buttons & 0x8000) != 0;

            // D-Pad uses a **4-bit value** (lower nibble of report[8])
            uint8_t dpad = buttons & 0x0F;
            state.buttons[static_cast<size_t>(Button::DPadUp)] = (dpad == 0 || dpad == 1 || dpad == 7);
            state.buttons[static_cast<size_t>(Button::DPadRight)] = (dpad == 1 || dpad == 2 || dpad == 3);
            state.buttons[static_cast<size_t>(Button::DPadDown)] = (dpad == 3 || dpad == 4 || dpad == 5);
            state.buttons[static_cast<size_t>(Button::DPadLeft)] = (dpad == 5 || dpad == 6 || dpad == 7);

            state.buttons[static_cast<size_t>(Button::PS)] = (report[10] & 0x01) != 0;
            state.buttons[static_cast<size_t>(Button::Touchpad)] = (report[10] & 0x02) != 0;
            state.buttons[static_cast<size_t>(Button::Mute)] = (report[10] & 0x04) != 0;
        }
        else if (reportID == 0x01 && !connectionType) {
            // The button bit field (3 bytes)
            uint32_t buttons = (report[8] << 0) | (report[9] << 8) | (report[10] << 16);

            // Map buttons based on known bit positions
            state.buttons[static_cast<size_t>(Button::Cross)] = (buttons & 0x20) != 0;
            state.buttons[static_cast<size_t>(Button::Circle)] = (buttons & 0x40) != 0;
            state.buttons[static_cast<size_t>(Button::Square)] = (buttons & 0x10) != 0;
            state.buttons[static_cast<size_t>(Button::Triangle)] = (buttons & 0x80) != 0;

            state.buttons[static_cast<size_t>(Button::L1)] = (buttons & 0x100) != 0;
            state.buttons[static_cast<size_t>(Button::R1)] = (buttons & 0x200) != 0;
            state.buttons[static_cast<size_t>(Button::L2)] = (buttons & 0x400) != 0;
            state.buttons[static_cast<size_t>(Button::R2)] = (buttons & 0x800) != 0;

            state.buttons[static_cast<size_t>(Button::Share)] = (buttons & 0x1000) != 0;
            state.buttons[static_cast<size_t>(Button::Options)] = (buttons & 0x2000) != 0;

            state.buttons[static_cast<size_t>(Button::L3)] = (buttons & 0x4000) != 0;
            state.buttons[static_cast<size_t>(Button::R3)] = (buttons & 0x8000) != 0;

            // D-Pad uses a **4-bit value** (lower nibble of report[8])
            uint8_t dpad = buttons & 0x0F;
            state.buttons[static_cast<size_t>(Button::DPadUp)] = (dpad == 0 || dpad == 1 || dpad == 7);
            state.buttons[static_cast<size_t>(Button::DPadRight)] = (dpad == 1 || dpad == 2 || dpad == 3);
            state.buttons[static_cast<size_t>(Button::DPadDown)] = (dpad == 3 || dpad == 4 || dpad == 5);
            state.buttons[static_cast<size_t>(Button::DPadLeft)] = (dpad == 5 || dpad == 6 || dpad == 7);

            state.buttons[static_cast<size_t>(Button::PS)] = (report[10] & 0x01) != 0;
            state.buttons[static_cast<size_t>(Button::Touchpad)] = (report[10] & 0x02) != 0;
            state.buttons[static_cast<size_t>(Button::Mute)] = (report[10] & 0x04) != 0;
            // I assume mute isn't supported...
            //state.buttons[static_cast<size_t>(Button::Mute)] = (report[10] & 0x04) != 0;
        }
        else if (reportID == 0x31 && !connectionType) {
            // The button bit field (3 bytes)
            uint32_t buttons = (report[9] << 0) | (report[10] << 8) | (report[11] << 16);

            // Map buttons based on known bit positions
            state.buttons[static_cast<size_t>(Button::Cross)] = (buttons & 0x20) != 0;
            state.buttons[static_cast<size_t>(Button::Circle)] = (buttons & 0x40) != 0;
            state.buttons[static_cast<size_t>(Button::Square)] = (buttons & 0x10) != 0;
            state.buttons[static_cast<size_t>(Button::Triangle)] = (buttons & 0x80) != 0;

            state.buttons[static_cast<size_t>(Button::L1)] = (buttons & 0x100) != 0;
            state.buttons[static_cast<size_t>(Button::R1)] = (buttons & 0x200) != 0;
            state.buttons[static_cast<size_t>(Button::L2)] = (buttons & 0x400) != 0;
            state.buttons[static_cast<size_t>(Button::R2)] = (buttons & 0x800) != 0;

            state.buttons[static_cast<size_t>(Button::Share)] = (buttons & 0x1000) != 0;
            state.buttons[static_cast<size_t>(Button::Options)] = (buttons & 0x2000) != 0;

            state.buttons[static_cast<size_t>(Button::L3)] = (buttons & 0x4000) != 0;
            state.buttons[static_cast<size_t>(Button::R3)] = (buttons & 0x8000) != 0;

            // D-Pad uses a **4-bit value** (lower nibble of report[8])
            uint8_t dpad = buttons & 0x0F;
            state.buttons[static_cast<size_t>(Button::DPadUp)] = (dpad == 0 || dpad == 1 || dpad == 7);
            state.buttons[static_cast<size_t>(Button::DPadRight)] = (dpad == 1 || dpad == 2 || dpad == 3);
            state.buttons[static_cast<size_t>(Button::DPadDown)] = (dpad == 3 || dpad == 4 || dpad == 5);
            state.buttons[static_cast<size_t>(Button::DPadLeft)] = (dpad == 5 || dpad == 6 || dpad == 7);

            state.buttons[static_cast<size_t>(Button::PS)] = (report[11] & 0x01) != 0;
            state.buttons[static_cast<size_t>(Button::Touchpad)] = (report[11] & 0x02) != 0;

            state.buttons[static_cast<size_t>(Button::Mute)] = (report[11] & 0x04) != 0;


        }
    }

    void Input::parseAnalogInputs(const std::vector<uint8_t>& report, InputState& state, bool connectionType, int reportID)
    {
        // Analog stick parsing
        if (connectionType && reportID == 0x01) {
            state.axes[static_cast<size_t>(Axis::LeftStickX)] =
                (report[0] - 127.0f) / 127.0f;
            state.axes[static_cast<size_t>(Axis::LeftStickY)] =
                (report[1] - 127.0f) / 127.0f;

            state.axes[static_cast<size_t>(Axis::RightStickX)] =
                (report[2] - 127.0f) / 127.0f;
            state.axes[static_cast<size_t>(Axis::RightStickY)] =
                (report[3] - 127.0f) / 127.0f;

            // Trigger parsing
            state.axes[static_cast<size_t>(Axis::L2Trigger)] =
                report[5] / 255.0f;
            state.axes[static_cast<size_t>(Axis::R2Trigger)] =
                report[6] / 255.0f;

            // L2
            state.triggers[0].inEffect = (report[42] && 0x10) != 0;
            state.triggers[0].state = (report[42] & 0x0F);

            // R2
            state.triggers[0].inEffect = (report[43] && 0x10) != 0;
            state.triggers[0].state = (report[43] & 0x0F);

        }
        else if (!connectionType && reportID == 0x01) {
            state.axes[static_cast<size_t>(Axis::LeftStickX)] =
                (report[0] - 127.0f) / 127.0f;
            state.axes[static_cast<size_t>(Axis::LeftStickY)] =
                (report[1] - 127.0f) / 127.0f;

            state.axes[static_cast<size_t>(Axis::RightStickX)] =
                (report[2] - 127.0f) / 127.0f;
            state.axes[static_cast<size_t>(Axis::RightStickY)] =
                (report[3] - 127.0f) / 127.0f;

            // Trigger parsing
            state.axes[static_cast<size_t>(Axis::L2Trigger)] =
                report[5] / 255.0f;
            state.axes[static_cast<size_t>(Axis::R2Trigger)] =
                report[6] / 255.0f;
        }
        else if (!connectionType && reportID == 0x31) {
            state.axes[static_cast<size_t>(Axis::LeftStickX)] =
                static_cast<float>((static_cast<double>(2) * report[2] / 0xFF) - 1.0);
            state.axes[static_cast<size_t>(Axis::LeftStickY)] =
                static_cast<float>((static_cast<double>(2) * report[3] / 0xFF) - 1.0);

            state.axes[static_cast<size_t>(Axis::RightStickX)] =
                static_cast<float>((static_cast<double>(2) * report[4] / 0xFF) - 1.0);
            state.axes[static_cast<size_t>(Axis::RightStickY)] =
                static_cast<float>((static_cast<double>(2) * report[5] / 0xFF) - 1.0);
            
            // Trigger parsing
            state.axes[static_cast<size_t>(Axis::L2Trigger)] =
                report[6] / 255.0f;
            state.axes[static_cast<size_t>(Axis::R2Trigger)] =
                report[7] / 255.0f;

            // L2
            state.triggers[0].inEffect = (report[44] && 0x10) != 0;
            state.triggers[0].state = (report[44] & 0x0F);

            // R2
            state.triggers[1].inEffect = (report[45] && 0x10) != 0;
            state.triggers[1].state = (report[46] & 0x0F);
        }
    }

    void Input::parseTouchpad(const std::vector<uint8_t>& report, InputState& state, bool connectionType, int reportID)
    {
        if (reportID == 0x01 && connectionType) {
            // Touchpad parsing
            state.touchpadPressed = (report[35] & 0x01) != 0;

            uint8_t touch00 = report[33];
            uint8_t touch01 = report[34];
            uint8_t touch02 = report[35];
            uint8_t touch03 = report[36];
            uint8_t touch10 = report[37];
            uint8_t touch11 = report[38];
            uint8_t touch12 = report[39];
            uint8_t touch13 = report[40];

            // First touch point
            state.touchPoints[0].active = !(touch00 & 0x80);
            state.touchPoints[0].x = ((touch02 & 0x0F) << 8) | touch01;
            state.touchPoints[0].y = (touch03 << 4) | ((touch02 & 0xF0) >> 4);
            state.touchPoints[0].id = (touch00 & 0x7F);

            // Second touch point
            state.touchPoints[1].active = !(touch10 & 0x80);
            state.touchPoints[1].x = (touch10 & 0x7F);
            state.touchPoints[1].y = ((touch12 & 0x0F) << 8) | touch11;
            state.touchPoints[1].id = (touch13 << 4) | ((touch12 & 0xF0) >> 4);
        }
        else if (reportID == 0x01 && !connectionType) { // NOT SUPPROTED ON BT Report 01 - Use 31
            return;
        }
        else if (reportID == 0x31 && !connectionType) {
            // Touchpad parsing
            state.touchpadPressed = (report[36] & 0x01) != 0;

            uint8_t touch00 = report[34];
            uint8_t touch01 = report[35];
            uint8_t touch02 = report[36];
            uint8_t touch03 = report[37];
            uint8_t touch10 = report[38];
            uint8_t touch11 = report[39];
            uint8_t touch12 = report[40];
            uint8_t touch13 = report[41];

            // First touch point
            state.touchPoints[0].active = !(touch00 & 0x80);
            state.touchPoints[0].x = ((touch02 & 0x0F) << 8) | touch01;
            state.touchPoints[0].y = (touch03 << 4) | ((touch02 & 0xF0) >> 4);
            state.touchPoints[0].id = (touch00 & 0x7F);

            // Second touch point
            state.touchPoints[1].active = !(touch10 & 0x80);
            state.touchPoints[1].x = ((touch12 & 0x0F) << 8) | touch11;
            state.touchPoints[1].y = (touch13 << 4) | ((touch12 & 0xF0) >> 4);
            state.touchPoints[1].id = (touch10 & 0x7F);
        }
    }

    void Input::parseMotion(const std::vector<uint8_t>& report, InputState& state, bool connectionType, int reportID)
    {
        if (connectionType && reportID == 0x01) {
            // Accelerometer data (little-endian)
            state.axes[static_cast<size_t>(Axis::AccelerometerX)] = static_cast<int16_t>(report[18] | (report[19] << 8)) / 8192.0f;
            state.axes[static_cast<size_t>(Axis::AccelerometerY)] = static_cast<int16_t>(report[16] | (report[17] << 8)) / 8192.0f;
            state.axes[static_cast<size_t>(Axis::AccelerometerZ)] = static_cast<int16_t>(report[20] | (report[21] << 8)) / 8192.0f;

            // Gyroscope data (little-endian)
            state.axes[static_cast<size_t>(Axis::GyroscopeX)] = static_cast<int16_t>(report[24] | (report[25] << 8)) / 1024.0f;
            state.axes[static_cast<size_t>(Axis::GyroscopeY)] = static_cast<int16_t>(report[27] | (report[27] << 8)) / 1024.0f;
            state.axes[static_cast<size_t>(Axis::GyroscopeZ)] = static_cast<int16_t>(report[26] | (report[29] << 8)) / 1024.0f;
        }
        else if (reportID == 0x01 && !connectionType) { // NOT SUPPROTED ON BT Report 01 - Use 31
            return;
        }
        else if (reportID == 0x31 && !connectionType) {
            // Accelerometer data (little-endian)
            state.axes[static_cast<size_t>(Axis::AccelerometerY)] = static_cast<int16_t>(report[17] | (report[18] << 8)) / 8192.0f;
            state.axes[static_cast<size_t>(Axis::AccelerometerX)] = static_cast<int16_t>(report[19] | (report[20] << 8)) / 8192.0f;
            state.axes[static_cast<size_t>(Axis::AccelerometerZ)] = static_cast<int16_t>(report[21] | (report[22] << 8)) / 8192.0f;

            // Gyroscope data (little-endian)
            state.axes[static_cast<size_t>(Axis::GyroscopeY)] = static_cast<int16_t>(report[23] | (report[24] << 8)) / 1024.0f;
            state.axes[static_cast<size_t>(Axis::GyroscopeX)] = static_cast<int16_t>(report[26] | (report[26] << 8)) / 1024.0f;
            state.axes[static_cast<size_t>(Axis::GyroscopeZ)] = static_cast<int16_t>(report[25] | (report[28] << 8)) / 1024.0f;
        }
    }


}