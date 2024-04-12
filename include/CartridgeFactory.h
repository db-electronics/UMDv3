#ifndef CARTRIDGEFACTORY_H
#define CARTRIDGEFACTORY_H

#include <Arduino.h>
#include <memory>

#include "cartridges/cartridge.h"
#include "cartridges/genesis.h"

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


#endif