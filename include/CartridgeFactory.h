#ifndef CARTRIDGEFACTORY_H
#define CARTRIDGEFACTORY_H

#include <Arduino.h>
#include "cartridges/cartridge.h"

class CartridgeFactory
{
    public:
        CartridgeFactory();
        ~CartridgeFactory();

        #define SYSTEMS_LEN 2
        enum System : uint8_t{
            UNDEFINED = 0x00,
            GENESIS   = 0x01
        };

        Cartridge* getCart(System mode);

    private:
        CartridgeFactory(const CartridgeFactory&) = delete;
        Cartridge* carts[SYSTEMS_LEN-1];
};

#endif