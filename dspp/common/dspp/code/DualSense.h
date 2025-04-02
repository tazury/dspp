#pragma once
#include "hidapi/include/hidapi.h" // Always use "" for .h - Use <> for .cpp
#include "DualSenseInput.h"
#include "DualSenseOutput.h"

namespace DualSense {
    struct CtrlInfo {
        hid_device* hid_dev;
        std::atomic<bool> is_connected;
        std::atomic<bool> is_checking_connection;
        bool is_usb;
        int number;
    };
    class DualSense {
    public:
        CtrlInfo controller;
        Input input = Input(controller);
        Output output = Output(controller);

    };
}