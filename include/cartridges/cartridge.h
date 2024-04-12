#ifndef CARTRIDGE_H
#define CARTRIDGE_H

#define DATA_BUFFER_SIZE_BYTES 512
#define UMD_MAX_RESULT_LINES 8
#define UMD_MAX_RESULT_LINE_LENGTH 32

#include <STM32SD.h>
#include "services/IChecksumCalculator.h"
#include "UMDPortsV3.h"
#include "umdDisplay.h"
#include "Menu.h"
#include <tuple>
#include <map>

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

struct CartridgeInfo{
    const char * Name;
    uint32_t Size;
};

enum UMDResultCode: int{
    FAIL = -1,
    OK,
    DISPLAYRESULT,
    DISPLAYMEMORIES,
    DISPLAYFILELIST,
    LOADMENU,
    WAITING
};

struct UMDActionResult{
    UMDMenuIndex NextMenu;
    UMDResultCode Code;
    const char * ErrorMessage;
    uint16_t ResultLines;
    char Result[UMD_MAX_RESULT_LINES][UMD_MAX_RESULT_LINE_LENGTH+1];

    UMDActionResult(){
        NextMenu = UMDMenuIndex::UMD_MENU_MAIN;
        Code = UMDResultCode::OK;
        ErrorMessage = nullptr;
        ResultLines = 0;
        for(int i = 0; i < UMD_MAX_RESULT_LINES; i++){
            Result[i][0] = '\0';
        }
    }
};

class Cartridge : public UMDPortsV3 {
    public:

        Cartridge(IChecksumCalculator& checksumCalculator);
        virtual ~Cartridge();
        void testWait(void);

        enum MemoryType : uint8_t{
            PRG0 = 0, PRG1, PRG2, PRG3,
            CHR0, CHR1,
            RAM0, RAM1,
            BRAM,
            EXT0, EXT1
        };

        enum CartridgeState : int8_t{
            IDLE=-1,
            IDENTIFY=0,
            READ,
            WRITE
        };

        enum ReadOptions : uint8_t{
            NONE = 0,
            HW_CHECKSUM,
            SYSTEM_CHECKUM
        };

        // Base functionality
        virtual void ResetChecksumCalculator();

        // Pure virtual functionality
        virtual void InitIO () = 0;
        virtual const char* GetSystemName() = 0;
        virtual uint32_t GetSize() = 0;
        virtual uint32_t ReadRom(uint32_t address, uint8_t *buffer, uint16_t size, ReadOptions opt) = 0;

        virtual UMDActionResult act(CartridgeState menuIndex, uint16_t menuItemIndex) = 0;

        virtual std::vector<const char *>& memoryGetNames() = 0;
        virtual int memoryRead(uint32_t address, uint8_t *buffer, uint16_t size, MemoryType mem) = 0;
        virtual int memoryWrite(uint32_t address, uint8_t *buffer, uint16_t size, MemoryType mem) = 0;
        virtual int memoryVerify(uint32_t address, uint8_t *buffer, uint16_t size, MemoryType mem) = 0;
        virtual int memoryChecksum(uint32_t address, uint32_t size, MemoryType mem, bool reset) = 0;

        virtual int flashErase(uint8_t mem) = 0;
        virtual int flashProgram(uint32_t address, uint8_t *buffer, uint16_t size, uint8_t mem) = 0;
        virtual FlashInfo flashGetInfo(uint8_t mem) = 0;
        virtual bool flashIsBusy(uint8_t mem) = 0;

        virtual void erasePrgFlash(bool wait);
        virtual uint8_t togglePrgBit(uint8_t attempts);
        virtual uint8_t readPrgByte(uint32_t address);
        virtual void readPrgBytes(uint32_t address, uint8_t *buffer, uint16_t size);
        virtual void writePrgByte(uint16_t address, uint8_t data);
        virtual uint16_t readPrgWord(uint32_t);
        virtual void writePrgWord(uint32_t, uint16_t);

        virtual void readPrgWords(uint32_t address, uint16_t *buffer, uint16_t size);

    protected:

        IChecksumCalculator& _checksumCalculator;

        File romFile;
        union {
            uint8_t byte[DATA_BUFFER_SIZE_BYTES];
            uint16_t word[DATA_BUFFER_SIZE_BYTES/2];
        } _dataBuffer;

        FlashInfo _flashInfo;
        Checksum _checksum;
        uint16_t ExpectedChecksum;
        uint16_t ActualChecksum;

        std::map<uint8_t, Cartridge::MemoryType> _memIndex;
        std::vector<const char *> _memNames;

        virtual FlashInfo getPrgFlashInfo();
        uint32_t getFlashSizeFromInfo(FlashInfo info);
        virtual bool calculateChecksum(uint32_t start, uint32_t end) = 0;
};

#endif