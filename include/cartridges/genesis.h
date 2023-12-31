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
        virtual const __FlashStringHelper** getMenuItems();
        virtual int getMenuSize() {return _menuSize;};

        virtual uint8_t readByte(uint32_t address);
        virtual void writeByte(uint16_t address, uint8_t data);
        virtual uint16_t readWord(uint32_t address);

    protected:
        const __FlashStringHelper* _menuTopLevel[10] = {F("Read"), F("Write"), F("Checksum"), F("Bob"), F("is"), F("a"), F("Fat"), F("Cookie"), F("Eating"), F("Whore")};
        const int _menuSize = 10;

};

#endif