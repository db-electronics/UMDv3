#ifndef I2CSCANNER_H
#define I2CSCANNER_H

#include <Wire.h>
#include <vector>

class I2CScanner
{
    public:
        std::vector<uint8_t> findDevices(TwoWire *wire = &Wire);
};

#endif