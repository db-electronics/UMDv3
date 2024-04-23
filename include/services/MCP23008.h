#pragma once

#include <Arduino.h>
#include <Wire.h>

#define MCP23008_GP0                     0b00000001
#define MCP23008_GP1                     0b00000010
#define MCP23008_GP2                     0b00000100
#define MCP23008_GP3                     0b00001000
#define MCP23008_GP4                     0b00010000
#define MCP23008_GP5                     0b00100000
#define MCP23008_GP6                     0b01000000
#define MCP23008_GP7                     0b10000000

namespace i2cdevice{
    class Mcp23008
    {
        public:

            enum INTCON_MODE
            {
                PREVIOUS = 0,
                DEFVAL = 1
            };

            bool begin(uint8_t address, TwoWire *wire = &Wire);
            bool pinMode(uint8_t pins, uint8_t mode);
            bool digitalWrite(uint8_t pins, uint8_t value);
            bool tooglePins(uint8_t pins);
            bool setPinPolarity(uint8_t pins, bool invert);
            bool setInterruptEnable(uint8_t pins, bool set);
            bool setInterruptOnChange(uint8_t pins, bool set);
            bool setDefaultValue(uint8_t pins, uint8_t value);
            bool setInterruptControl(uint8_t pins, INTCON_MODE mode);
            bool setPullUpResistors(uint8_t pins, bool set);
            uint8_t readInterruptFlags();
            uint8_t readInterruptCapture();
            uint8_t readGPIO();

            enum Pins : uint8_t{
                GP0 = 0b00000001,
                GP1 = 0b00000010,
                GP2 = 0b00000100,
                GP3 = 0b00001000,
                GP4 = 0b00010000,
                GP5 = 0b00100000,
                GP6 = 0b01000000,
                GP7 = 0b10000000
            };

        private:
            const uint8_t MCP23008_BASE_ADDRESS = 0x20;
            const uint8_t MCP23008_MAX_ADDRESS = 0x27;
            uint8_t mError = 0;

            uint8_t mDeviceAddress;
            TwoWire *pWire;

            std::array<uint8_t, 11> mRegisters = {0};

            bool Init(void);

            /// @brief perform read-modify-write operation on a register
            /// @param registerAddress 
            /// @param bitMask 
            /// @param set 
            /// @return 
            bool UpdateRegister(uint8_t registerAddress, uint8_t bitMask, bool set);

            /// @brief write a value to a register
            /// @param registerAddress 
            /// @param val 
            /// @return 
            bool WriteRegister(uint8_t registerAddress, uint8_t val);

            /// @brief 
            /// @param registerAddress 
            /// @return 
            uint8_t ReadRegister(uint8_t registerAddress);

            enum class Registers : uint8_t{
                IODIR = 0x00,
                IPOL = 0x01,
                GPINTEN = 0x02,
                DEFVAL = 0x03,
                INTCON = 0x04,
                IOCON = 0x05,
                GPPU = 0x06,
                INTF = 0x07,
                INTCAP = 0x08,
                GPIO = 0x09,
                OLAT = 0x0A
            };

            constexpr uint8_t GetRegisterAddress(Registers reg){
                return static_cast<uint8_t>(reg);
            }
    };
}

