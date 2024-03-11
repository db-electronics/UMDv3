#ifndef GENESIS_H
#define GENESIS_H

#include "IUMDPorts.h"
#include "cartridge.h"
#include "Menu.h"
#include <string>

#define GENESIS_HEADER_SIZE             256
#define GENESIS_HEADER_START_ADDR       0x00000100
#define GENESIS_HEADER_ROM_START_ADDR   0x000001A0
#define GENESIS_HEADER_ROM_END_ADDR     0x000001A4

struct GenesisHeader{
    union {
        struct{
            char SystemType[16];
            char Copyright[16];
            char DomesticName[48];
            char InternationalName[48];
            char SerialNumber[14];
            uint16_t Checksum;
            char DeviceSupport[16];
            uint32_t ROMStart;
            uint32_t ROMEnd;
            uint32_t RAMStart;
            uint32_t RAMEnd;
            char MemoryType[2];
            uint8_t RAMType;
            uint8_t RAM20;
            uint32_t SRAMStart;
            uint32_t SRAMEnd;
            char ModemSupport[12];
            char Notes[40];
            char RegionSupport[3];
            char Reserved[13];
        };
        struct{
            uint8_t bytes[256];
        };
        struct{
            uint16_t words[128];
        };
    };
};

class Genesis : public Cartridge {
    public:

        Genesis();
        virtual ~Genesis();

        virtual void initIO();
        virtual const char* getSystemName();
        virtual std::tuple<const __FlashStringHelper**, uint16_t> getMenu(uint16_t id);
        virtual uint16_t doAction(uint16_t menuIndex, uint16_t menuItemIndex, const SDClass& sd);

        virtual bool calculateChecksum(uint32_t start, uint32_t end);

        virtual uint8_t readByte(uint32_t address);
        virtual void writeByte(uint16_t address, uint8_t data);
        virtual uint16_t readWord(uint32_t address);

    protected:

        GenesisHeader _header;

        bool readHeader();

        uint32_t readLong(uint32_t address);
        uint32_t getRomSizeFromHeader();


        // rename Genesis CE pins
        __attribute__((always_inline)) void setTIME() { setCE0(); }
        __attribute__((always_inline)) void setAS() { setCE1(); }
        __attribute__((always_inline)) void setLWR() { setCE2(); }
        __attribute__((always_inline)) void setCE() { setCE3(); }

        __attribute__((always_inline)) void clearTIME() { clearCE0(); }
        __attribute__((always_inline)) void clearAS() { clearCE1(); }
        __attribute__((always_inline)) void clearLWR() { clearCE2(); }
        __attribute__((always_inline)) void clearCE() { clearCE3(); }

        // DTACK on IO0
        __attribute__((always_inline)) void setDTACK() { setIO(0); }
        __attribute__((always_inline)) void clearDTACK() { clearIO(0); }

        // VRES on IO1
        __attribute__((always_inline)) void setVRES() { setIO(1); }
        __attribute__((always_inline)) void clearVRES() { clearIO(1); }

        // M3 on IO2
        __attribute__((always_inline)) void readM3() { ioRead(2); }

        // ASEL on IO3
        __attribute__((always_inline)) void setASEL() { setIO(3); }
        __attribute__((always_inline)) void clearASEL() { clearIO(3); }

        // MRES on IO8
        __attribute__((always_inline)) void readMRES() { ioRead(8); }

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