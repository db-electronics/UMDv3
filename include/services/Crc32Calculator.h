#pragma once

#include "IChecksumCalculator.h"
#include <stm32f4xx_hal.h>
#include <stm32f4xx_hal_crc.h>

class Crc32Calculator : public IChecksumCalculator
{
public:
    Crc32Calculator();
    virtual ~Crc32Calculator();
    void Reset() override;
    uint32_t Accumulate(uint32_t pBuffer[], uint32_t length) override;
    uint32_t Get() override;
};