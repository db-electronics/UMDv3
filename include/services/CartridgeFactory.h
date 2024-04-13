#pragma once

#include <Arduino.h>
#include <memory>

#include "cartridges/Cartridge.h"
#include "cartridges/Genesis.h"

class CartridgeFactory
{
    public:
        // these must match the ids returned by the cartridge adapters
        enum UMDAdapterType : uint8_t{
            UNDEFINED = 0x00,
            GENESIS   = 0x01
        };

        std::unique_ptr<Cartridge> GetCart(uint8_t adapterId, IChecksumCalculator& checksumCalculator);
};
