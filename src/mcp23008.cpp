#include "mcp23008.h"

bool MCP23008::begin(uint8_t address = 0x20, TwoWire *wire = &Wire)
{
    if ((address >= 0x20) && (address <= 0x27)) {
        _i2cAddress = address;
    } else if (address <= 0x07) {
        _i2cAddress = 0x20 + address;
    } else {
        _i2cAddress = 0x27;
    }

    return _initAllPOR();
}

bool MCP23008::_initAllPOR(void)
{
    // these should be the POR values in the device
    _registers[MCP23008_IODIR] = 0xFF;
    for(int i = 1; i < MCP23008_NUM_OF_SHADOW_REGISTERS; i++){
        _registers[i] = 0;
    }

    _wire->beginTransmission(_i2cAddress);
    _wire->write(MCP23008_IODIR);
    _wire->write(_registers, MCP23008_NUM_OF_SHADOW_REGISTERS);
    _wire->endTransmission();

    return true;
}

bool MCP23008::_writeRegister(uint8_t registerAddress, uint8_t val)
{
    _wire->beginTransmission(_i2cAddress);
    _wire->write(registerAddress);
    _wire->write(val);
    _wire->endTransmission();

    return true;
}

uint8_t MCP23008::_readRegister(uint8_t registerAddress)
{
    uint8_t readValue;

    _wire->beginTransmission(_i2cAddress);
    _wire->write(registerAddress);
    readValue = _wire->read();
    _wire->endTransmission();
    return readValue;
}

bool MCP23008::pinMode(uint8_t pins, uint8_t mode)
{
    // 1 = input in MCP23008
    if(mode == INPUT)
    {
        _registers[MCP23008_IODIR] |= mode;
    }
    else
    {
        _registers[MCP23008_IODIR] &= ~mode;
    }

    return _writeRegister(MCP23008_IODIR, _registers[MCP23008_IODIR]);
}

bool MCP23008::digitalWrite(uint8_t pins, uint8_t value)
{
    // 1 = input in MCP23008
    if(value == LOW)
    {
        _registers[MCP23008_OLAT] &= ~pins;
    }
    else
    {
        _registers[MCP23008_OLAT] |= pins;
    }

    return _writeRegister(MCP23008_OLAT, _registers[MCP23008_OLAT]);
}

bool MCP23008::setPinPolarity(uint8_t pins, bool invert)
{
    // 1 = input in MCP23008
    if(invert)
    {
        _registers[MCP23008_IPOL] |= pins;
    }
    else
    {
        _registers[MCP23008_IPOL] &= ~pins;
    }

    return _writeRegister(MCP23008_IPOL, _registers[MCP23008_IPOL]);
}