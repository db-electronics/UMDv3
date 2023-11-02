#ifndef CARTRIDGEFACTORY_H
#define CARTRIDGEFACTORY_H

#include <Arduino.h>
#include <memory>

#include "cartridges/UMDPortsV3.h"
#include "cartridges/cartridge.h"
#include "cartridges/genesis.h"

// Hardware abstraction for ports

class CartridgeFactory
{
    public:
        // these to match with the ids returned by the cartridge adapters
        enum UMDAdapterType : uint8_t{
            UNDEFINED = 0x00,
            GENESIS   = 0x01
        };

        std::unique_ptr<Cartridge> getCart(uint8_t adapterId);
};

//IUMDPorts* CartridgeFactory::_ports = new UMDPortsV3();

#endif