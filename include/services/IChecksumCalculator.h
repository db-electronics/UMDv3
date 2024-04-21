#pragma once

#include <cstdint>

class IChecksumCalculator
{
public:
    virtual void Reset() = 0;
    // TODO return an array of uint32_t to allow for larger hash results
    virtual uint32_t Accumulate(uint32_t pBuffer[], uint32_t length) = 0;
    virtual uint32_t Get() = 0;
};