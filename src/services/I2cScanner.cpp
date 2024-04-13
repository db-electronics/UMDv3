
#include "services/I2cScanner.h"

std::vector<uint8_t> I2cScanner::FindDevices(TwoWire *wire)
{
    std::vector<uint8_t> addresses;
    uint8_t error;

    for(int address = 1; address < 127; address++)
    {
        wire->beginTransmission(address);
        error = wire->endTransmission();
        if(error == 0)
        {
            addresses.push_back(address);
        }
    }

    return addresses;
}