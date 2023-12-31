#ifndef GENESIS_H
#define GENESIS_H

#include "IUMDPorts.h"
#include "cartridge.h"
#include "Menu.h"
#include <string>


class Genesis : public Cartridge {
    public:

        Genesis();
        virtual ~Genesis();
        virtual const char* getSystemName();

        // virtual const __FlashStringHelper** getMenuItems(int id);
        // virtual int getMenuSize(int id);
        virtual std::tuple<const __FlashStringHelper**, int> getMenu(int id);

        virtual uint8_t readByte(uint32_t address);
        virtual void writeByte(uint16_t address, uint8_t data);
        virtual uint16_t readWord(uint32_t address);

    protected:
        //const __FlashStringHelper* _menuItems[2] = {F("Read"), F("Write")};
        //Menu<2> _bad {{F("Read"), F("Write")}};

        // MAIN MENU
        const __FlashStringHelper* _mainMenuItems[10] = {F("Read"), F("Write"), F("Checksum"), F("Bob"), F("is"), F("a"), F("Fat"), F("Cookie"), F("Eating"), F("Whore")};
        Menu<10> _mainMenu = _mainMenuItems;
};

#endif