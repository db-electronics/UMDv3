#ifndef CARTRIDGE_H
#define CARTRIDGE_H

#include "IUMDPorts.h"
#include <string>

class Cartridge {
    public:
        Cartridge(IUMDPorts *umdPorts);
        virtual ~Cartridge();

        const String SystemName;

        virtual uint8_t readByte(uint16_t address);
        virtual uint8_t readByte(uint32_t address);

    protected:
        const uint16_t addressSetupTime = 100;
        const uint16_t readHoldTime = 200;

    private:
        IUMDPorts *_umdPorts;
};

#endif