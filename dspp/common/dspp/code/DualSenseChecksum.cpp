
#include "DualSenseChecksum.h"

	void DualSense::DualSenseChecksum::fillChecksum(uint8_t reportId, std::array<uint8_t, 78>& reportData) {
        const auto crc = calculateCrc32({ 0xA2, reportId }, reportData.data() + 1, reportData.size() - 5);

        // Write CRC in little-endian format
        reportData[reportData.size() - 4] = (crc >> 0) & 0xFF;
        reportData[reportData.size() - 3] = (crc >> 8) & 0xFF;
        reportData[reportData.size() - 2] = (crc >> 16) & 0xFF;
        reportData[reportData.size() - 1] = (crc >> 24) & 0xFF;
    }
    const std::array<uint32_t, 256>& DualSense::DualSenseChecksum::getCRCTable()
    {
        static const auto crcTable = makeCRCTable();
        return crcTable;
    }

    std::array<uint32_t, 256> DualSense::DualSenseChecksum::makeCRCTable()
    {
        std::array<uint32_t, 256> table{};
        for (uint32_t n = 0; n < 256; ++n) {
            uint32_t c = n;
            for (int k = 0; k < 8; ++k) {
                if (c & 1) {
                    c = 0xEDB88320 ^ (c >> 1);
                }
                else {
                    c >>= 1;
                }
            }
            table[n] = c;
        }
        return table;
    }

    uint32_t DualSense::DualSenseChecksum::calculateCrc32(const std::vector<uint8_t>& prefix, const uint8_t* data, size_t dataLength)
    {
        const auto& crcTable = getCRCTable();
        uint32_t crc = 0xFFFFFFFF;

        // Process prefix bytes
        for (const auto byte : prefix) {
            crc = (crc >> 8) ^ crcTable[(crc ^ byte) & 0xFF];
        }

        // Process main data
        for (size_t i = 0; i < dataLength; ++i) {
            crc = (crc >> 8) ^ crcTable[(crc ^ data[i]) & 0xFF];
        }

        return crc ^ 0xFFFFFFFF;
    }

        