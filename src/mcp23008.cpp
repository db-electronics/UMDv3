#include "mcp23008.h"

bool MCP23008::begin(uint8_t address, TwoWire *wire)
{
    if ((address >= MCP23008_BASE_ADDRESS) && (address <= MCP23008_MAX_ADDRESS)) {
        _deviceAddress = address;
    } else if (address <= 0x07) {
        _deviceAddress = MCP23008_BASE_ADDRESS + address;
    } else {
        _deviceAddress = MCP23008_MAX_ADDRESS;
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

    _wire->beginTransmission(_deviceAddress);
    _wire->write(MCP23008_IODIR);
    _wire->write(_registers, MCP23008_NUM_OF_SHADOW_REGISTERS);
    error = _wire->endTransmission();
    if(error == 0){
        return true;    
    }
    return false;
}

bool MCP23008::_writeRegister(uint8_t registerAddress, uint8_t val)
{
    _wire->beginTransmission(_deviceAddress);
    _wire->write(registerAddress);
    _wire->write(val);
    error = _wire->endTransmission();
    if(error == 0){
        return true;    
    }
    return false;
}

uint8_t MCP23008::_readRegister(uint8_t registerAddress)
{
    uint8_t readValue;

    _wire->beginTransmission(_deviceAddress);
    _wire->write(registerAddress);
    readValue = _wire->read();
    error = _wire->endTransmission();
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

bool MCP23008::tooglePins(uint8_t pins)
{
    _registers[MCP23008_OLAT] ^= pins;
    return _writeRegister(MCP23008_OLAT, _registers[MCP23008_OLAT]);
}

bool MCP23008::setPinPolarity(uint8_t pins, bool invert)
{
    // 1 = pin polarity is inverted
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

bool MCP23008::setInterruptOnChange(uint8_t pins, bool set)
{
    // 1 = pin is configured for interrupt on change
    if(set)
    {
        _registers[MCP23008_GPINTEN] |= pins;
    }
    else
    {
        _registers[MCP23008_GPINTEN] &= ~pins;
    }

    return _writeRegister(MCP23008_GPINTEN, _registers[MCP23008_GPINTEN]);
}