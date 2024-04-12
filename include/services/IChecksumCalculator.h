#pragma once

#include <cstdint>

class IChecksumCalculator
{
public:
    virtual void Reset() = 0;
    virtual uint32_t Accumulate(uint32_t pBuffer[], uint32_t length) = 0;
};