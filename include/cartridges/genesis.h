#ifndef GENESIS_H
#define GENESIS_H

#include "IUMDPorts.h"
#include "cartridge.h"
#include <string>


template<int size_t>
class UMDMenu
{
    public:
        const int Size = size_t;
        const __FlashStringHelper* Items[size_t];
        UMDMenu(const __FlashStringHelper* items[size_t])
        {
            for(int i = 0; i < size_t; i++)
            {
                Items[i] = items[i];
            }
        }
};

class Genesis : public Cartridge {
    public:

        Genesis();
        virtual ~Genesis();
        virtual const char* getSystemName();

        virtual const __FlashStringHelper** getMenuItems(int id);
        virtual int getMenuSize(int id);

        virtual uint8_t readByte(uint32_t address);
        virtual void writeByte(uint16_t address, uint8_t data);
        virtual uint16_t readWord(uint32_t address);

    protected:
        // const __FlashStringHelper* _menuTopLevel[10] = {F("Read"), F("Write"), F("Checksum"), F("Bob"), F("is"), F("a"), F("Fat"), F("Cookie"), F("Eating"), F("Whore")};
        // const int _menuSize = 10;

        std::array<const __FlashStringHelper*, 10> _myMenu = {F("Read"), F("Write"), F("Checksum"), F("Bob"), F("is"), F("a"), F("Fat"), F("Cookie"), F("Eating"), F("Whore")};

        const __FlashStringHelper* _menuItems[2] = {F("Read"), F("Write")};
        const __FlashStringHelper* _mainMenuItems[10] = {F("Read"), F("Write"), F("Checksum"), F("Bob"), F("is"), F("a"), F("Fat"), F("Cookie"), F("Eating"), F("Whore")};
        //UMDMenu<2> _bad = {F("Read"), F("Write")};
        UMDMenu<10> _mainMenu = _mainMenuItems;
};






#endif