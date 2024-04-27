#pragma once

#define DATA_BUFFER_SIZE_BYTES 512

#include <vector>
#include <string>
#include <cstring>
#include <map>

#include "services/IChecksumCalculator.h"
#include "Array.h"
#include "memory/FlashInfo.h"
#include "UMDPortsV3.h"

namespace cartridges{

    class Cartridge : public UMDPortsV3 {
    public:

        Cartridge(IChecksumCalculator& checksumCalculator);
        virtual ~Cartridge();
        void TestWait(void);

        enum class MemoryType : uint8_t{
            PRG0 = 0, PRG1, PRG2, PRG3,
            CHR0, CHR1,
            RAM0, RAM1,
            BRAM,
            EXT0, EXT1
        };

        /// @brief Options for reading the ROM
        enum ReadOptions : uint8_t{
            NONE = 0,
            CHECKSUM_CALCULATOR
        };

        /// @brief Reset the checksum calculator
        void ResetChecksumCalculator();
        std::vector<const char *>& GetMemoryNames() { return mMemoryNames; };
        std::vector<const char *>& GetMetadata() { return mMetadata; };
        uint32_t GetAccumulatedChecksum() { return mChecksumCalculator.Get(); };

        /// @brief Initialize the IO for the system
        virtual void InitIO () = 0;
        
        /// @brief Get the name of the system 
        virtual const std::string& GetSystemName() const = 0;

        /// @brief Get the name of the cartridge currently connected, if it is knowable via the header
        virtual const char* GetCartridgeName() = 0;

        /// @brief Get the size of the cartridge currently connected, if it is knowable via the header
        virtual uint32_t GetCartridgeSize() = 0;

        virtual FlashInfo GetFlashInfo(uint8_t memTypeIndex) = 0;
        virtual int EraseFlash(uint8_t memTypeIndexm) = 0;

        /// @brief Identify the cartridge by reading all its data and computing a checksum
        /// @param address The start address to read from
        /// @param buffer The buffer to read into
        /// @param size The number of bytes to read
        /// @param opt The options for reading
        /// @return The accumulated checksum
        virtual uint32_t Identify(uint32_t address, cartridges::Array& array, ReadOptions opt) = 0;

        virtual uint32_t ReadMemory(uint32_t address, cartridges::Array& array, uint8_t memTypeIndex, ReadOptions opt) = 0;

        virtual int ProgramFlash(uint32_t address, uint8_t *buffer, uint16_t size, uint8_t memTypeIndex) = 0;
    
        virtual bool IsFlashBusy(uint8_t memTypeIndex) = 0;

    protected:

        IChecksumCalculator& mChecksumCalculator;
        std::map<uint8_t, Cartridge::MemoryType> mMemoryTypeIndexMap;
        std::vector<const char *> mMemoryNames;
        std::vector<const char *> mMetadata;

        bool IsMemoryIndexValid(uint8_t memTypeIndex) const {
            return memTypeIndex < mMemoryNames.size();
        }
    };
}