#pragma once

#include <Arduino.h>
#include <Wire.h>

#define MCP23008_BASE_ADDRESS            0x20 //!< MCP23008 serial address
#define MCP23008_MAX_ADDRESS             0x27

// registers
#define MCP23008_IODIR                   0x00 //!< I/O direction register
#define MCP23008_IPOL                    0x01 //!< Input polarity register
#define MCP23008_GPINTEN                 0x02 //!< Interrupt-on-change control register
#define MCP23008_DEFVAL                  0x03 //!< Default compare register for interrupt-on-change
#define MCP23008_INTCON                  0x04 //!< Interrupt control register
#define MCP23008_IOCON                   0x05 //!< Configuration register
#define MCP23008_GPPU                    0x06 //!< Pull-up resistor configuration register
#define MCP23008_INTF                    0x07 //!< Interrupt flag register
#define MCP23008_INTCAP                  0x08 //!< Interrupt capture register
#define MCP23008_GPIO                    0x09 //!< Port register
#define MCP23008_OLAT                    0x0A //!< Output latch register
#define MCP23008_NUM_OF_SHADOW_REGISTERS 11

#define MCP23008_GP0                     0b00000001
#define MCP23008_GP1                     0b00000010
#define MCP23008_GP2                     0b00000100
#define MCP23008_GP3                     0b00001000
#define MCP23008_GP4                     0b00010000
#define MCP23008_GP5                     0b00100000
#define MCP23008_GP6                     0b01000000
#define MCP23008_GP7                     0b10000000

class MCP23008
{
    public:
        /// @brief the last error
        uint8_t error;
        enum INTCON_MODE
        {
            PREVIOUS = 0,
            DEFVAL = 1
        };

        /// @brief initialize the mcp23008 at the specified address
        /// @param address 7 bit device address in the range 0x20 - 0x27
        /// @param wire
        /// @return
        bool begin(uint8_t address = MCP23008_BASE_ADDRESS, TwoWire *wire = &Wire);
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

    private:
        uint8_t mDeviceAddress;
        TwoWire *pWire;
        uint8_t mRegisters[MCP23008_NUM_OF_SHADOW_REGISTERS];

        bool _initAllPOR(void);
        bool _updateRegister(uint8_t registerAddress, uint8_t bitMask, bool set);
        bool _writeDeviceRegister(uint8_t registerAddress, uint8_t val);
        uint8_t _readDeviceRegister(uint8_t registerAddress);
};
