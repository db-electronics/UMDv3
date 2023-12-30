#ifndef CARTRIDGE_H
#define CARTRIDGE_H

#include "UMDPortsV3.h"

class Cartridge : public UMDPortsV3 {
    public:

        Cartridge();
        virtual ~Cartridge();
        void testWait(void);

        virtual const __FlashStringHelper** getMenuItems() = 0;
        virtual int getMenuSize() = 0;

        virtual const char* getSystemName() = 0;
        virtual uint8_t readByte(uint16_t address);
        virtual uint8_t readByte(uint32_t address);
        virtual void readBytes(uint32_t address, uint8_t *buffer, uint16_t size);

        virtual void writeByte(uint16_t address, uint8_t data);

        virtual uint16_t readWord(uint32_t);

    protected:
        const uint16_t addressSetupTime = 100;
        const uint16_t readHoldTime = 200;
        const uint16_t writeHoldTime = 200;

        // const __FlashStringHelper * menuTopLevel[3] = {F("Read Cartridge"), F("Write Cartridge"), F("Checksum")};
        // const int menuTopLevelSize = 3;

};


// Given the following templated class, how can I declare a new instance and
// initialize it with the following values?
// {("Read Cartridge"), ("Write Cartridge"), ("Checksum")};



#endif