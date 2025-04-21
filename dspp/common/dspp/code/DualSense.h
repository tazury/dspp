/*
* This is an common file, meaning that this is used in all platforms. So don't stress anything.
* It is recommanded to build from source if you have an older version but note the gcc version.
 *
* Title: DS++ DualSense Class
* Author: Munashe Dirwayi [wanomaniac]
* Purpose: "The root class for both controller information, input and output of the DualSense controller
           This is the main way of accessing most of the library."

* Compatibility: Designed in C++ 20 | C Standard 17
* Updates -
          ** UPDATES WILL BE HERE **

* Known issues -
          ** BUGS WILL BE REPORTED HERE **
*/
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