#ifndef CARTRIDGE_H
#define CARTRIDGE_H

#include "umdPorts.h"

class Cartridge : public umdPorts{
    public:
        Cartridge();
        virtual ~Cartridge();

        virtual uint8_t readByte(uint16_t address);
    protected:
        uint16_t ticks;
};

#endif