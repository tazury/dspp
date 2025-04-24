/*
* This is an common file, meaning that this is used in all platforms. So don't stress anything.

* Title: DS++ DualSense Bluetooth Checksum
* Author: Munashe Dirwayi [wanomaniac]
* Purpose: "This allows us to send an output to an bluetooth controller, without this. It wouldn't work.
            There is no need to use this class in your code."

* Compatibility: Designed in C++ 20 | C Standard 17 

* Updates -
          ** UPDATES WILL BE HERE **

* Known issues -
          ** BUGS WILL BE REPORTED HERE **
*/
#pragma once
#include <array>
#include <vector>
#include <cstdint>

namespace DualSense {
    class DualSenseChecksum {
    public:
        static void fillChecksum(uint8_t reportId, std::array<uint8_t, 78>& reportData);
    private:
        static const std::array<uint32_t, 256>& getCRCTable();
        static std::array<uint32_t, 256> makeCRCTable();
        static uint32_t calculateCrc32(const std::vector<uint8_t>& prefix,
            const uint8_t* data,
            size_t dataLength);
    };
}