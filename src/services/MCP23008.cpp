#include "services/MCP23008.h"

bool MCP23008::begin(uint8_t address, TwoWire *wire)
{
    // keep address in range
    if ((address >= MCP23008_BASE_ADDRESS) && (address <= MCP23008_MAX_ADDRESS))
    {
        mDeviceAddress = address;
    }
    else if (address <= 0x07)
    {
        mDeviceAddress = MCP23008_BASE_ADDRESS + address;
    }
    else
    {
        mDeviceAddress = MCP23008_MAX_ADDRESS;
    }

    pWire = wire;

    // check if device is actually there
    pWire->beginTransmission(mDeviceAddress);
    error = pWire->endTransmission();

    if (error != 0)
    {
        return false;
    }

    return _initAllPOR();
}

bool MCP23008::_initAllPOR(void)
{
    // these should be the POR values in the device
    mRegisters[MCP23008_IODIR] = 0xFF;
    for (int i = 1; i < MCP23008_NUM_OF_SHADOW_REGISTERS; i++)
    {
        mRegisters[i] = 0;
    }

    pWire->beginTransmission(mDeviceAddress);
    pWire->write(MCP23008_IODIR);
    pWire->write(mRegisters, MCP23008_NUM_OF_SHADOW_REGISTERS);
    error = pWire->endTransmission();
    if (error == 0)
    {
        return true;
    }
    return false;
}

bool MCP23008::_writeDeviceRegister(uint8_t registerAddress, uint8_t val)
{
    pWire->beginTransmission(mDeviceAddress);
    pWire->write(registerAddress);
    pWire->write(val);
    error = pWire->endTransmission();
    if (error == 0)
    {
        return true;
    }
    return false;
}

bool MCP23008::_updateRegister(uint8_t registerAddress, uint8_t bitMask, bool set)
{
    if (set)
    {
        mRegisters[registerAddress] |= bitMask;
    }
    else
    {
        mRegisters[registerAddress] &= ~bitMask;
    }

    return _writeDeviceRegister(registerAddress, mRegisters[registerAddress]);
}

uint8_t MCP23008::_readDeviceRegister(uint8_t registerAddress)
{
    uint8_t readValue;

    pWire->beginTransmission(mDeviceAddress);
    pWire->write(registerAddress);
    pWire->endTransmission(false);
    pWire->requestFrom(mDeviceAddress, 1, true);
    readValue = pWire->read();
    error = pWire->endTransmission();
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
    mRegisters[MCP23008_OLAT] ^= (pins & ~mRegisters[MCP23008_IODIR]);
    return _writeDeviceRegister(MCP23008_OLAT, mRegisters[MCP23008_OLAT]);
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