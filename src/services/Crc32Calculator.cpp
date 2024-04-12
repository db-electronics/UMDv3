#include "services/Crc32Calculator.h"

Crc32Calculator::Crc32Calculator()
{
    // enable CRC unit
    __HAL_RCC_CRC_CLK_ENABLE();
}

Crc32Calculator::~Crc32Calculator()
{
    // disable CRC unit
    __HAL_RCC_CRC_CLK_DISABLE();
}

void Crc32Calculator::Reset()
{
    // reset CRC unit
    __HAL_RCC_CRC_CLK_ENABLE();
    CRC->CR = CRC_CR_RESET;
}

// copied from HAL lib because this is disabled in platformio libs :(
uint32_t Crc32Calculator::Accumulate(uint32_t pBuffer[], uint32_t length)
{
    uint32_t crc = 0U;  /* CRC output (read from hcrc->Instance->DR register) */
    for (int i = 0; i < length; i++)
    {
        CRC->DR = pBuffer[i];
    }
    crc = CRC->DR;

    /* Return the CRC computed value */
    return crc;

    // Pat Riley GM MK-1201 -00 HW CRC32 calculation = 38E025EB
    // Uses CRC-32 (Ethernet) polynomial: 0x4C11DB7
    // https://community.st.com/t5/stm32-mcus/how-is-crc-value-calculated-how-to-determine-crc-algorithm-in/ta-p/49719
    // CRC_CR: 0x0000 0000; POLYSIZE is 32, no REV_IN and no REV_OUT
    // CRC_INIT: 0XFFFF FFFF
    // CRC_POLY: 0X04D11 CDB7
    // https://community.st.com/t5/stm32-mcus/how-is-crc-value-calculated-how-to-determine-crc-algorithm-in/ta-p/49719#_msocom_1
}