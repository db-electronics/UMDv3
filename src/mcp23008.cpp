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

    return _initAllInputs();
}

bool MCP23008::_initAllInputs(void)
{
    // these should be the POR values in the device
    uint8_t buffer[] = {MCP23008_IODIR, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

    _wire->beginTransmission(_i2cAddress);
    _wire->write(buffer, 10);
    _wire->endTransmission();

    return true;
}

void MCP23008::writeRegister(uint8_t registerAddress, uint8_t val)
{
    _wire->beginTransmission(_i2cAddress);
    _wire->write(registerAddress);
    _wire->write(val);
    _wire->endTransmission();
}

uint8_t MCP23008::readRegister(uint8_t registerAddress)
{
    uint8_t readValue;

    _wire->beginTransmission(_i2cAddress);
    _wire->write(registerAddress);
    readValue = _wire->read();
    _wire->endTransmission();
    return readValue;
}

void MCP23008::pinMode(uint8_t pins, uint8_t mode)
{
    // read the current value
    uint8_t currentIoDir = readRegister(MCP23008_IODIR);

    // 1 = input in MCP23008
    if(mode == INPUT)
    {
        currentIoDir |= mode;
    }
    else
    {
        currentIoDir &= ~mode;
    }

    _wire->beginTransmission(_i2cAddress);
    _wire->write(MCP23008_IODIR);
    _wire->write(currentIoDir);
    _wire->endTransmission();
}