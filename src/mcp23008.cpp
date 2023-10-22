#include "mcp23008.h"

bool MCP23008::begin(uint8_t address, TwoWire *wire)
{
    // keep address in range
    if ((address >= MCP23008_BASE_ADDRESS) && (address <= MCP23008_MAX_ADDRESS)) {
        _deviceAddress = address;
    } else if (address <= 0x07) {
        _deviceAddress = MCP23008_BASE_ADDRESS + address;
    } else {
        _deviceAddress = MCP23008_MAX_ADDRESS;
    }

    _wire = wire;

    // check if device is actually there
    _wire->beginTransmission(_deviceAddress);
    error = _wire->endTransmission();
    
    if(error != 0)
    {
        return false;
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

bool MCP23008::_writeDeviceRegister(uint8_t registerAddress, uint8_t val)
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

bool MCP23008::_updateRegister(uint8_t registerAddress, uint8_t bitMask, bool set)
{
    if(set)
    {
        _registers[registerAddress] |= bitMask;
    }
    else
    {
        _registers[registerAddress] &= ~bitMask;
    }

    return _writeDeviceRegister(registerAddress, _registers[registerAddress]);
}

uint8_t MCP23008::_readDeviceRegister(uint8_t registerAddress)
{
    uint8_t readValue;

    _wire->beginTransmission(_deviceAddress);
    _wire->write(registerAddress);
    _wire->endTransmission(false);
    _wire->requestFrom(_deviceAddress, 1, true);
    readValue = _wire->read();
    error = _wire->endTransmission();
    return readValue;
}

bool MCP23008::pinMode(uint8_t pins, uint8_t mode)
{
    // 1 = input in MCP23008
    bool set = mode == OUTPUT ? false : true;
    return _updateRegister(MCP23008_IODIR, pins, set);
}

bool MCP23008::digitalWrite(uint8_t pins, uint8_t value)
{
    bool set = value == LOW ? false : true;
    return _updateRegister(MCP23008_OLAT, pins, set);
}

bool MCP23008::tooglePins(uint8_t pins)
{
    // only toggle the output pins, so we don't create garbage data in the shadow registers
    _registers[MCP23008_OLAT] ^= (pins & ~_registers[MCP23008_IODIR]);
    return _writeDeviceRegister(MCP23008_OLAT, _registers[MCP23008_OLAT]);
}

bool MCP23008::setPinPolarity(uint8_t pins, bool invert)
{
    // 1 = pin polarity is inverted
    return _updateRegister(MCP23008_IPOL, pins, invert);
}

bool MCP23008::setInterruptEnable(uint8_t pins, bool set)
{
    // 1 = Enable GPIO input pin for interrupt-on-change event
    return _updateRegister(MCP23008_GPINTEN, pins, set);
}

bool MCP23008::setInterruptOnChange(uint8_t pins, bool set)
{
    // 1 = pin is configured for interrupt on change
    return _updateRegister(MCP23008_INTCON, pins, set);
}

bool MCP23008::setDefaultValue(uint8_t pins, uint8_t value)
{
    // if the associated pin level is the opposite from the register bit, an interrupt occurs.
    bool set = value == LOW ? false : true;
    return _updateRegister(MCP23008_DEFVAL, pins, set);
}

bool MCP23008::setInterruptControl(uint8_t pins, INTCON_MODE mode)
{
    // if the associated pin level is the opposite from the register bit, an interrupt occurs.
    bool set = mode == DEFVAL ? true : false;
    return _updateRegister(MCP23008_INTCON, pins, set);
}

bool MCP23008::setPullUpResistors(uint8_t pins, bool set)
{
    // 1 = pullup is enabled
    return _updateRegister(MCP23008_GPPU, pins, set);
}

uint8_t MCP23008::readInterruptFlags()
{
    // 1 = pin caused an interrupt
    return _readDeviceRegister(MCP23008_INTF);
}

uint8_t MCP23008::readInterruptCapture()
{
    // state of pins when interrupt occured
    return _readDeviceRegister(MCP23008_INTCAP);
}

uint8_t MCP23008::readGPIO()
{
    return _readDeviceRegister(MCP23008_GPIO);
}