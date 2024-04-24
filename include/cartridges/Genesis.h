#pragma once

#include "Cartridge.h"
#include "GenesisHeader.h"
#include <string>

#define GENESIS_HEADER_ROM_START_ADDR   0x000001A0
#define GENESIS_HEADER_ROM_END_ADDR     0x000001A4

#define GENESIS_HEADER_SIZE_OF_SYSTEM_TYPE 16
#define GENESIS_HEADER_SIZE_OF_COPYRIGHT 16
#define GENESIS_HEADER_SIZE_OF_DOMESTIC_NAME 48
#define GENESIS_HEADER_SIZE_OF_INTERNATIONAL_NAME 48
#define GENESIS_HEADER_SIZE_OF_SERIAL_NUMBER 14

class Genesis : public Cartridge {
    public:

        Genesis(IChecksumCalculator& checksumCalculator);
        virtual ~Genesis();

        virtual void InitIO() override;
        virtual const std::string& GetSystemName() const {return mSystemName;};
        virtual const char* GetCartridgeName() override;
        virtual uint32_t GetCartridgeSize() override;

        virtual FlashInfo GetFlashInfo(uint8_t memTypeIndex) override;
        virtual int EraseFlash(uint8_t memTypeIndex) override;
        virtual uint32_t Identify(uint32_t address, uint8_t *buffer, uint16_t size, ReadOptions opt) override;

        virtual uint32_t ReadMemory(uint32_t address, uint8_t *buffer, uint16_t size, uint8_t memTypeIndex, ReadOptions opt) override;

        // TODO REMOVE THIS METHOD
        int doAction(uint16_t menuIndex, uint16_t menuItemIndex, const SDClass& sd, UMDDisplay& disp);

        virtual CartridgeActionResult act(CartridgeState menuIndex, uint16_t menuItemIndex);

        virtual int flashProgram(uint32_t address, uint8_t *buffer, uint16_t size, uint8_t mem);
        virtual bool flashIsBusy(uint8_t mem);

        virtual uint8_t togglePrgBit(uint8_t attempts);

        virtual uint8_t readPrgByte(uint32_t address);
        virtual void writePrgByte(uint32_t address, uint8_t data);

        virtual void readPrgWords(uint32_t address, uint16_t *buffer, uint16_t size);

    protected:

        virtual bool calculateChecksum(uint32_t start, uint32_t end);
        
    private:

        genesis::Header mHeader;
        const std::string mSystemName = "MD";
        const uint32_t HEADER_START_ADDR = 0x00000100;
        const uint32_t HEADER_SIZE = 256;
        const uint32_t TIME_CONFIG_ADDR = 0xA130F1;

        void ReadHeader();
        uint16_t ReadPrgWord(uint32_t address);
        void WritePrgWord(uint32_t address, uint16_t data);
        
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
