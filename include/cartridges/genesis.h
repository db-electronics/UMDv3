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
        virtual std::tuple<const __FlashStringHelper**, uint16_t> getMenu(uint16_t id);
        virtual uint16_t doAction(uint16_t menuIndex, uint16_t menuItemIndex, const SDClass& sd);

        virtual uint8_t readByte(uint32_t address);
        virtual void writeByte(uint16_t address, uint8_t data);
        virtual uint16_t readWord(uint32_t address);

    protected:
        //const __FlashStringHelper* _menuItems[2] = {F("Read"), F("Write")};
        //Menu<2> _bad {{F("Read"), F("Write")}};

        // 0 - MAIN MENU
        #define GENESIS_MAIN_MENU_SIZE 3
        const __FlashStringHelper* _mainMenuItems[GENESIS_MAIN_MENU_SIZE] = {F("Read"), F("Write"), F("Checksum")};
        Menu<GENESIS_MAIN_MENU_SIZE> _mainMenu = _mainMenuItems;

        // 1 - READ MENU
        #define GENESIS_READ_MENU_SIZE 2
        const __FlashStringHelper* _readMenuItems[GENESIS_READ_MENU_SIZE] = {F("Dump ROM"), F("Dump RAM")};
        Menu<GENESIS_READ_MENU_SIZE> _readMenu = _readMenuItems;

        // 2 - WRITE MENU
        #define GENESIS_WRITE_MENU_SIZE 2
        const __FlashStringHelper* _writeMenuItems[GENESIS_WRITE_MENU_SIZE] = {F("Write ROM"), F("Write RAM")};
        Menu<GENESIS_WRITE_MENU_SIZE> _writeMenu = _writeMenuItems;

        // 3 - Checksum MENU
        #define GENESIS_CHECKSUM_MENU_SIZE 1
        const __FlashStringHelper* _checksumMenuItems[GENESIS_CHECKSUM_MENU_SIZE] = {F("Verify Checksum")};
        Menu<GENESIS_CHECKSUM_MENU_SIZE> _checksumMenu = _checksumMenuItems;
};

#endif