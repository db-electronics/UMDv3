#pragma once

#include <Arduino.h>
#include <memory>

#include "cartridges/Cartridge.h"
#include "services/IChecksumCalculator.h"
#include "services/Crc32Calculator.h"
#include "cartridges/Genesis/Genesis.h"

class CartridgeFactory
{
    public:
        // these must match the ids returned by the cartridge adapters
        enum UMDAdapterType : uint8_t{
            UNDEFINED = 0x00,
            GENESIS   = 0x01,
            SMS       = 0x02,
            SNES      = 0x03,
            GBA       = 0x04,
            GBC       = 0x05
        };

        std::unique_ptr<Cartridge> GetCart(uint8_t adapterId);

    private:
        // TODO make this a smart pointer
        Crc32Calculator crc32Calculator;
};
