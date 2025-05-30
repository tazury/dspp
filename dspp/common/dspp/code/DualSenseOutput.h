/*
* This is an common file, meaning that this is used in all platforms. So don't stress anything.

* Title: DS++ DualSense Output
* Author: Munashe Dirwayi [wanomaniac]
* Purpose: "This allows you to send an output to change the controller appearance (Like colors, etc)
           Utilize the examples to understand this well"

* Compatibility: Designed in C++ 20 | C Standard 17

* Updates -
          ** UPDATES WILL BE HERE **

* Known issues -
          ** BUGS WILL BE REPORTED HERE **
*/
#pragma once
#include <cstdint>
#include <array>
#include <atomic>
#include <functional>
#include "hidapi/include/hidapi.h"
#include <mutex>
#include <thread>
#include <vector>
#include "DualSenseChecksum.h"

namespace DualSense {
    struct CtrlInfo;
    enum  PlayerLED : uint8_t {
        None = 0,  // No LED
        Player1 = 0x04,
        Player2 = 0x01,
        Player3 = 0x05,
        Player4 = 0x13,
    };
    class Output {
    public:
        uint8_t validityFlag = 0xff - 8;
        bool bluetoothF = false;
        CtrlInfo& controller;
        int outputSeq_ = 1;

        Output(CtrlInfo& ctrl) : controller(ctrl) {}

        // Constructor
        void Reset(bool useFlags = true);

        // LED configuration
        struct LedSettings {
            bool muteLED = false;
            //uint8_t option = 0x00;
            //uint8_t brightness = 0x00;
            //uint8_t pulseOption = 0x00;
            uint8_t playerNumber = 0x00;
            struct {
                uint8_t red = 0x00;
                uint8_t green = 0x00;
                uint8_t blue = 0x00;
            } colors;
        };

        // Trigger configuration
        struct TriggerSettings {
            uint8_t mode = 0x00;
            uint8_t force1 = 0x00;
            uint8_t force2 = 0x00;
            uint8_t force3 = 0x00;
            uint8_t force4 = 0x00;
            uint8_t force5 = 0x00;
            uint8_t force6 = 0x00;
            uint8_t force7 = 0x00;
        };

        // Motor configuration
        uint8_t overallMotors = 0x00;
        uint8_t leftMoter = 0x00;
        uint8_t rightMoter = 0x00;

    public:
         uint8_t getLEDNumber();
        int sendOutput();
    public:  
        // Public members
        LedSettings ledSettings;
        TriggerSettings leftTrigger;
        TriggerSettings rightTrigger;

    private:
        int sendOutputUSB();
        int sendOutputBT();
        void setTriggerUSB(std::array<uint8_t, 48>& report, const TriggerSettings& trigger, int offset);
        void setTriggerUSB(std::array<uint8_t, 47>& report, const TriggerSettings& trigger, int offset);
        void setTriggerBT(std::array<uint8_t, 78>& report, const TriggerSettings& trigger, int offset);
    };
}