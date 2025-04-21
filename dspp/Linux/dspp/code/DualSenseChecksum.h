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