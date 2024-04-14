#pragma once

#define DATA_BUFFER_SIZE_BYTES 512
#define UMD_MAX_RESULT_LINES 8
#define UMD_MAX_RESULT_LINE_LENGTH 32

#include <tuple>
#include <map>

#include <STM32SD.h>
#include "services/IChecksumCalculator.h"
#include "services/UmdDisplay.h"
#include "memory/FlashInfo.h"
#include "UMDPortsV3.h"

enum CartridgeResultCode: int{
    FAIL = -1,
    OK,
    DISPLAYRESULT,
    DISPLAYMEMORIES,
    DISPLAYFILELIST,
    LOADMENU,
    WAITING
};

struct CartridgeActionResult{
    CartridgeResultCode Code;
    const char * ErrorMessage;
    uint16_t ResultLines;
    char Result[UMD_MAX_RESULT_LINES][UMD_MAX_RESULT_LINE_LENGTH+1];

    CartridgeActionResult(){
        Code = CartridgeResultCode::OK;
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
        void TestWait(void);

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

        /// @brief Options for reading the ROM
        enum ReadOptions : uint8_t{
            NONE = 0,
            HW_CHECKSUM,
            SYSTEM_CHECKUM
        };

        /// @brief Reset the checksum calculator
        void ResetChecksumCalculator();
        std::vector<const char *>& GetMemoryNames() { return mMemoryNames; };
        std::vector<const char *>& GetMetadata() { return mMetadata; };
        uint32_t GetAccumulatedChecksum() { return mChecksumCalculator.Get(); };

        /// @brief Initialize the IO for the system
        virtual void InitIO () = 0;
        
        /// @brief Get the name of the system 
        virtual const char* GetSystemName() = 0;

        /// @brief Get the name of the cartridge currently connected, if it is knowable via the header
        virtual const char* GetCartridgeName() = 0;

        /// @brief Get the size of the cartridge currently connected, if it is knowable via the header
        virtual uint32_t GetCartridgeSize() = 0;

        virtual FlashInfo GetFlashInfo(MemoryType mem) = 0;
        virtual int EraseFlash(MemoryType mem) = 0;

        /// @brief Identify the cartridge by reading all its data and computing a checksum
        /// @param address The start address to read from
        /// @param buffer The buffer to read into
        /// @param size The number of bytes to read
        /// @param opt The options for reading
        /// @return The accumulated checksum
        virtual uint32_t Identify(uint32_t address, uint8_t *buffer, uint16_t size, ReadOptions opt) = 0;

        virtual void ReadMemory(uint32_t address, uint8_t *buffer, uint16_t size, MemoryType mem, ReadOptions opt) = 0;

        virtual int flashProgram(uint32_t address, uint8_t *buffer, uint16_t size, uint8_t mem) = 0;
    
        virtual bool flashIsBusy(uint8_t mem) = 0;


    protected:

        IChecksumCalculator& mChecksumCalculator;
        std::map<uint8_t, Cartridge::MemoryType> mMemoryIndexToType;
        std::vector<const char *> mMemoryNames;
        std::vector<const char *> mMetadata;

        File romFile;
        union {
            uint8_t byte[DATA_BUFFER_SIZE_BYTES];
            uint16_t word[DATA_BUFFER_SIZE_BYTES/2];
        } _dataBuffer;

        uint16_t ExpectedChecksum;
        uint16_t ActualChecksum;
};
