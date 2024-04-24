#pragma once

#include <cstddef>
#include <cstdint>

namespace genesis{

    constexpr size_t SYSTEM_TYPE_SIZE = 16;
    constexpr size_t COPYRIGHT_SIZE = 16;
    constexpr size_t DOMESTIC_NAME_SIZE = 48;
    constexpr size_t INTERNATIONAL_NAME_SIZE = 48;
    constexpr size_t SERIAL_NUMBER_SIZE = 14;

    struct Header{
        union {
            struct{
                char SystemType[SYSTEM_TYPE_SIZE];
                char Copyright[COPYRIGHT_SIZE];
                char DomesticName[DOMESTIC_NAME_SIZE];
                char InternationalName[INTERNATIONAL_NAME_SIZE];
                char SerialNumber[SERIAL_NUMBER_SIZE];
                uint16_t Checksum;
                char DeviceSupport[16];
                uint32_t ROMStart;
                uint32_t ROMEnd;
                uint32_t RAMStart;
                uint32_t RAMEnd;
                char MemoryType[2];
                uint8_t RAMType;
                uint8_t RAM20;
                uint32_t SRAMStart;
                uint32_t SRAMEnd;
                char ModemSupport[12];
                char Notes[40];
                char RegionSupport[3];
                char Reserved[13];
            };
            struct{
                uint8_t bytes[256];
            };
            struct{
                uint16_t words[128];
            };
        };

        struct {
            char SystemType[SYSTEM_TYPE_SIZE+1];
            char Copyright[COPYRIGHT_SIZE+1];
            char DomesticName[DOMESTIC_NAME_SIZE+1];
            char InternationalName[INTERNATIONAL_NAME_SIZE+1];
            char SerialNumber[SERIAL_NUMBER_SIZE+1];
        } Printable;
    };
}