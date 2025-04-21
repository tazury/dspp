#pragma once
#include <cstdint>
#include <array>
#include <atomic>
#include <functional>
#include "hidapi/include/hidapi.h"
#include <mutex>
#include <thread>
#include <vector>

namespace DualSense {
    struct CtrlInfo;

    class Input {
    public:
        std::atomic<bool> running;
        bool isMuted = false;
        bool connection = false;
        CtrlInfo& controller;

        enum class Button {
            Cross, Circle, Square, Triangle,
            L1, R1, L2, R2, Share, Options,
            L3, R3, PS, Touchpad,
            DPadUp, DPadDown, DPadLeft, DPadRight, Mute,
            _Count
        };

        enum class Axis {
            LeftStickX, LeftStickY,
            RightStickX, RightStickY,
            L2Trigger, R2Trigger,
            AccelerometerX, AccelerometerY, AccelerometerZ,
            GyroscopeX, GyroscopeY, GyroscopeZ,
            _Count
        };

        struct TouchPoint {
            bool active;
            long x;
            long y;
            uint8_t id;
        };

        struct TriggerInfo {
            bool inEffect;
            uint8_t state;
        };

        struct MotionState {
            float accelerometer[3]; // In G-forces
            float gyroscope[3];     // In degrees per second
        };

        struct ControllerInfo {
            uint8_t batteryLevel;

        };

        struct InputState {
            // Digital buttons
            bool buttons[static_cast<size_t>(Button::_Count)];

            // Analog axes
            float axes[static_cast<size_t>(Axis::_Count)];

            // Trigger Info
            TriggerInfo triggers[2];

            // Touchpad
            TouchPoint touchPoints[2]; // DualSense supports 2 simultaneous touches
            bool touchpadPressed;

            // Motion
            MotionState motion;

            // Microphone
            bool microphoneMuted;
        };
        using Listener = std::function<void(const InputState&, double timestamp)>;
    private:
        hid_device* device;
        // bool running;
        std::thread workerThread;
        std::vector<Listener> listeners;
        std::mutex listenersMutex;
    public:

        Input(CtrlInfo& ctrl) : controller(ctrl), device(nullptr), connection(false) {}
        ~Input();

        void addListener(Listener listener);

        bool start();

        void stop();

private: // Parsing Input
        void notifyListeners(const InputState& state, double timestamp);

        InputState parseReport(const std::vector<uint8_t>& report);

        static void parseButtons(const std::vector<uint8_t>& report, InputState& state, bool connectionType, int reportID);

        static void parseAnalogInputs(const std::vector<uint8_t>& report, InputState& state, bool connectionType, int reportID);

        void parseTouchpad(const std::vector<uint8_t>& report, InputState& state, bool connectionType, int reportID);

        void parseMotion(const std::vector<uint8_t>& report, InputState& state, bool connectionType, int reportID);
    };
}
