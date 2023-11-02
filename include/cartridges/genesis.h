#ifndef GENESIS_H
#define GENESIS_H

#include "IUMDPorts.h"
#include "cartridge.h"
#include <string>

class Genesis : public Cartridge {
    public:

        Genesis();
        virtual ~Genesis();


        virtual const char* getSystemName();

        virtual uint8_t readByte(uint32_t address);
        virtual void writeByte(uint16_t address, uint8_t data);
        virtual uint16_t readWord(uint32_t address);

};

#endif