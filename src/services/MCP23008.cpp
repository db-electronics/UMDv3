#include "services/Mcp23008.h"

bool i2cdevice::Mcp23008::begin(uint8_t address, TwoWire *wire)
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
    mError = pWire->endTransmission();

    if (mError != 0)
    {
        return false;
    }

    return Init();
}

bool i2cdevice::Mcp23008::Init(void)
{
    // these should be the POR values in the device
    std::fill(mRegisters.begin(), mRegisters.end(), 0);
    mRegisters[GetRegisterAddress(Registers::IODIR)] = 0xFF;

    pWire->beginTransmission(mDeviceAddress);
    pWire->write(GetRegisterAddress(Registers::IODIR));
    pWire->write(mRegisters.data(), mRegisters.size());
    mError = pWire->endTransmission();
    if (mError == 0)
    {
        return true;
    }
    return false;
}

bool i2cdevice::Mcp23008::WriteRegister(uint8_t registerAddress, uint8_t val)
{
    pWire->beginTransmission(mDeviceAddress);
    pWire->write(registerAddress);
    pWire->write(val);
    mError = pWire->endTransmission();
    if (mError == 0)
    {
        return true;
    }
    return false;
}

bool i2cdevice::Mcp23008::UpdateRegister(uint8_t registerAddress, uint8_t bitMask, bool set)
{
    if (set)
    {
        mRegisters[registerAddress] |= bitMask;
    }
    else
    {
        mRegisters[registerAddress] &= ~bitMask;
    }

    return WriteRegister(registerAddress, mRegisters[registerAddress]);
}

uint8_t i2cdevice::Mcp23008::ReadRegister(uint8_t registerAddress)
{
    uint8_t readValue;

    pWire->beginTransmission(mDeviceAddress);
    pWire->write(registerAddress);
    pWire->endTransmission(false);
    pWire->requestFrom(mDeviceAddress, 1, 1);
    readValue = pWire->read();
    mError = pWire->endTransmission();
    return readValue;
}

bool i2cdevice::Mcp23008::pinMode(uint8_t pins, uint8_t mode)
{
    // 1 = input in MCP23008
    bool set = mode == OUTPUT ? false : true;
    return UpdateRegister(GetRegisterAddress(Registers::IODIR), pins, set);
}

bool i2cdevice::Mcp23008::digitalWrite(uint8_t pins, uint8_t value)
{
    bool set = value == LOW ? false : true;
    return UpdateRegister(GetRegisterAddress(Registers::OLAT), pins, set);
}

bool i2cdevice::Mcp23008::tooglePins(uint8_t pins)
{
    // only toggle the output pins, so we don't create garbage data in the shadow registers
    mRegisters[GetRegisterAddress(Registers::OLAT)] ^= (pins & ~mRegisters[GetRegisterAddress(Registers::IODIR)]);
    return WriteRegister(GetRegisterAddress(Registers::OLAT), mRegisters[GetRegisterAddress(Registers::OLAT)]);
}

bool i2cdevice::Mcp23008::setPinPolarity(uint8_t pins, bool invert)
{
    // 1 = pin polarity is inverted
    return UpdateRegister(GetRegisterAddress(Registers::IPOL), pins, invert);
}

bool i2cdevice::Mcp23008::setInterruptEnable(uint8_t pins, bool set)
{
    // 1 = Enable GPIO input pin for interrupt-on-change event
    return UpdateRegister(GetRegisterAddress(Registers::GPINTEN), pins, set);
}

bool i2cdevice::Mcp23008::setInterruptOnChange(uint8_t pins, bool set)
{
    // 1 = pin is configured for interrupt on change
    return UpdateRegister(GetRegisterAddress(Registers::INTCON), pins, set);
}

bool i2cdevice::Mcp23008::setDefaultValue(uint8_t pins, uint8_t value)
{
    // if the associated pin level is the opposite from the register bit, an interrupt occurs.
    bool set = value == LOW ? false : true;
    return UpdateRegister(GetRegisterAddress(Registers::DEFVAL), pins, set);
}

bool i2cdevice::Mcp23008::setInterruptControl(uint8_t pins, INTCON_MODE mode)
{
    // if the associated pin level is the opposite from the register bit, an interrupt occurs.
    bool set = mode == DEFVAL ? true : false;
    return UpdateRegister(GetRegisterAddress(Registers::INTCON), pins, set);
}

bool i2cdevice::Mcp23008::setPullUpResistors(uint8_t pins, bool set)
{
    // 1 = pullup is enabled
    return UpdateRegister(GetRegisterAddress(Registers::GPPU), pins, set);
}

uint8_t i2cdevice::Mcp23008::readInterruptFlags()
{
    // 1 = pin caused an interrupt
    return ReadRegister(GetRegisterAddress(Registers::INTF));
}

uint8_t i2cdevice::Mcp23008::readInterruptCapture()
{
    // state of pins when interrupt occured
    return ReadRegister(GetRegisterAddress(Registers::INTCAP));
}

uint8_t i2cdevice::Mcp23008::readGPIO()
{
    return ReadRegister(GetRegisterAddress(Registers::GPIO));
}