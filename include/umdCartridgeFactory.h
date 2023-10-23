#ifndef UMD_H
#define UMD_H

#include <Arduino.h>
#include "cartridges/cartridge.h"

class UMDCartridgeFactory
{
    public:
        UMDCartridgeFactory();
        ~UMDCartridgeFactory();

        #define SYSTEMS_LEN 2
        enum System : uint8_t{
            UNDEFINED = 0x00,
            GENESIS   = 0x01
        };

        Cartridge* getCart(System mode);

    private:
        UMDCartridgeFactory(const UMDCartridgeFactory&) = delete;
        Cartridge* carts[SYSTEMS_LEN-1];
};

#endif