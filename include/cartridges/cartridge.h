#ifndef CARTRIDGE_H
#define CARTRIDGE_H

#include <STM32SD.h>
#include "UMDPortsV3.h"
#include <tuple>

class Cartridge : public UMDPortsV3 {
    public:

        Cartridge();
        virtual ~Cartridge();
        void testWait(void);

        virtual const char* getSystemName() = 0;
        virtual std::tuple<const __FlashStringHelper**, uint16_t> getMenu(uint16_t id) = 0;
        virtual uint16_t doAction(uint16_t menuIndex, uint16_t menuItemIndex, const SDClass& sd) = 0;
        
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


#endif