#ifndef CARTRIDGE_H
#define CARTRIDGE_H

#define DATA_BUFFER_SIZE_BYTES 512

#include <STM32SD.h>
#include "UMDPortsV3.h"
#include "umdDisplay.h"
#include <tuple>

struct FlashInfo{
    uint16_t Manufacturer;
    uint16_t Device;
    uint32_t Size;
};

struct Checksum{
    uint64_t Expected64;
    uint64_t Actual64;
    uint32_t Expected32;
    uint32_t Actual32;
    uint16_t Expected16;
    uint16_t Actual16;

    void Clear(){
        Actual64 = 0;
        Actual32 = 0;
        Actual16 = 0;
    }
};

class Cartridge : public UMDPortsV3 {
    public:

        Cartridge();
        virtual ~Cartridge();
        void testWait(void);

        virtual void initIO() = 0;
        virtual const char* getSystemName() = 0;
        virtual std::tuple<const __FlashStringHelper**, uint16_t> getMenu(uint16_t id) = 0;
        virtual int doAction(uint16_t menuIndex, uint16_t menuItemIndex, const SDClass& sd, UMDDisplay& disp) = 0;
        
        enum MemoryType : uint8_t{
            PRG0 = 0, PRG1, PRG2, PRG3,
            CHR0, CHR1,
            RAM0, RAM1,
            BRAM,
            EXT0, EXT1
        };

        virtual int memoryGetCount() = 0;
        virtual std::vector<MemoryType> memoryGetSupportedTypes() = 0;
        virtual const char* memoryGetName(uint8_t mem) = 0;
        virtual int memoryRead(uint32_t address, uint8_t *buffer, uint16_t size, uint8_t mem) = 0;
        virtual int memoryWrite(uint32_t address, uint8_t *buffer, uint16_t size, uint8_t mem) = 0;
        virtual int memoryVerify(uint32_t address, uint8_t *buffer, uint16_t size, uint8_t mem) = 0;
        virtual int memoryChecksum(uint32_t address, uint32_t size, uint8_t mem, bool reset) = 0;

        virtual int flashErase(bool wait, uint8_t mem) = 0;
        virtual int flashProgram(uint32_t address, uint8_t *buffer, uint16_t size, uint8_t mem) = 0;
        virtual int flashInfo(uint8_t mem) = 0;

        virtual void erasePrgFlash(bool wait);
        virtual uint8_t togglePrgBit(uint8_t attempts);
        virtual uint8_t readPrgByte(uint32_t address);
        virtual void readPrgBytes(uint32_t address, uint8_t *buffer, uint16_t size);
        virtual void writePrgByte(uint16_t address, uint8_t data);
        virtual uint16_t readPrgWord(uint32_t);
        virtual void writePrgWord(uint32_t, uint16_t);

        virtual void readPrgWords(uint32_t address, uint16_t *buffer, uint16_t size);

    protected:

        File romFile;
        union {
            uint8_t byte[DATA_BUFFER_SIZE_BYTES];
            uint16_t word[DATA_BUFFER_SIZE_BYTES/2];
        } _dataBuffer;

        FlashInfo _flashInfo;
        Checksum _checksum;
        uint16_t ExpectedChecksum;
        uint16_t ActualChecksum;

        virtual FlashInfo getPrgFlashInfo();
        uint32_t getFlashSizeFromInfo(FlashInfo info);
        virtual bool calculateChecksum(uint32_t start, uint32_t end) = 0;

        // const __FlashStringHelper * menuTopLevel[3] = {F("Read Cartridge"), F("Write Cartridge"), F("Checksum")};
        // const int menuTopLevelSize = 3;

};


#endif