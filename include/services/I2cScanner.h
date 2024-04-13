#pragma once

#include <Wire.h>
#include <vector>

class I2cScanner
{
    public:
        std::vector<uint8_t> FindDevices(TwoWire *wire = &Wire);
};
