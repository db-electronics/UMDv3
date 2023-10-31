#ifndef CARTRIDGE_H
#define CARTRIDGE_H

#include "UMDPortsV3.h"
#include <string>

class Cartridge : public UMDPortsV3 {
    public:

        Cartridge();
        virtual ~Cartridge();

        virtual const char* getSystemName() = 0;

        virtual uint8_t readByte(uint16_t address);
        virtual uint8_t readByte(uint32_t address);

        virtual void writeByte(uint16_t address, uint8_t data);

        virtual uint16_t readWord(uint32_t);

    protected:
        const uint16_t addressSetupTime = 100;
        const uint16_t readHoldTime = 200;
        const uint16_t writeHoldTime = 200;

};

#endif