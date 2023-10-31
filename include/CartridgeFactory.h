#ifndef CARTRIDGEFACTORY_H
#define CARTRIDGEFACTORY_H

#include <Arduino.h>
#include "cartridges/UMDPortsV3.h"
#include "cartridges/cartridge.h"
#include "cartridges/genesis.h"

// Hardware abstraction for ports

class CartridgeFactory
{
    public:
        CartridgeFactory(){};

        // these to match with the ids returned by the cartridge adapters
        enum System : uint8_t{
            UNDEFINED = 0x00,
            GENESIS   = 0x01
        };

        Cartridge* getCart(uint8_t system){

            switch(system){
                case GENESIS:
                    return new Genesis();
                default:
                    return nullptr;
            }
            return nullptr;
        }
};

//IUMDPorts* CartridgeFactory::_ports = new UMDPortsV3();

#endif