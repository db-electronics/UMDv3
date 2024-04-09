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



#define GENESIS_HEADER_SIZE_OF_SYSTEM_TYPE 16
#define GENESIS_HEADER_SIZE_OF_COPYRIGHT 16
#define GENESIS_HEADER_SIZE_OF_DOMESTIC_NAME 48
#define GENESIS_HEADER_SIZE_OF_INTERNATIONAL_NAME 48
#define GENESIS_HEADER_SIZE_OF_SERIAL_NUMBER 14


struct GenesisHeader{
    union {
        struct{
            char SystemType[GENESIS_HEADER_SIZE_OF_SYSTEM_TYPE];
            char Copyright[GENESIS_HEADER_SIZE_OF_COPYRIGHT];
            char DomesticName[GENESIS_HEADER_SIZE_OF_DOMESTIC_NAME];
            char InternationalName[GENESIS_HEADER_SIZE_OF_INTERNATIONAL_NAME];
            char SerialNumber[GENESIS_HEADER_SIZE_OF_SERIAL_NUMBER];
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

    struct {
        char SystemType[GENESIS_HEADER_SIZE_OF_SYSTEM_TYPE+1];
        char Copyright[GENESIS_HEADER_SIZE_OF_COPYRIGHT+1];
        char DomesticName[GENESIS_HEADER_SIZE_OF_DOMESTIC_NAME+1];
        char InternationalName[GENESIS_HEADER_SIZE_OF_INTERNATIONAL_NAME+1];
        char SerialNumber[GENESIS_HEADER_SIZE_OF_SERIAL_NUMBER+1];
    } Printable;
};

class Genesis : public Cartridge {
    public:

        Genesis();
        virtual ~Genesis();

        virtual void initIO();
        virtual const char* getSystemName();
        virtual int doAction(uint16_t menuIndex, uint16_t menuItemIndex, const SDClass& sd, UMDDisplay& disp);

        virtual UMDActionResult act(UMDMenuIndex menuIndex, uint16_t menuItemIndex);

        virtual std::vector<const char *>& memoryGetNames();
        virtual int memoryRead(uint32_t address, uint8_t *buffer, uint16_t size, uint8_t mem);
        virtual int memoryWrite(uint32_t address, uint8_t *buffer, uint16_t size, uint8_t mem);
        virtual int memoryVerify(uint32_t address, uint8_t *buffer, uint16_t size, uint8_t mem);
        virtual int memoryChecksum(uint32_t address, uint32_t size, uint8_t mem, bool reset);

        virtual int flashErase(uint8_t mem);
        virtual int flashProgram(uint32_t address, uint8_t *buffer, uint16_t size, uint8_t mem);
        virtual FlashInfo flashGetInfo(uint8_t mem);
        virtual bool flashIsBusy(uint8_t mem);

        virtual void erasePrgFlash(bool wait);
        virtual uint8_t togglePrgBit(uint8_t attempts);

        virtual uint8_t readPrgByte(uint32_t address);
        virtual void writePrgByte(uint32_t address, uint8_t data);

        virtual uint16_t readPrgWord(uint32_t address);
        virtual void writePrgWord(uint32_t address, uint16_t data);

        virtual void readPrgWords(uint32_t address, uint16_t *buffer, uint16_t size);

    protected:

        GenesisHeader _header;

        virtual FlashInfo getPrgFlashInfo();
        virtual bool calculateChecksum(uint32_t start, uint32_t end);
        
    private:

        const uint32_t _timeConfigAddr = 0xA130F1;

        bool readHeader();

        void enableSram(bool enable);

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

};

#endif