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

uint32_t Crc32Calculator::Get()
{
    return CRC->DR;
}

uint32_t Crc32Calculator::Accumulate(uint32_t pBuffer[], uint32_t length)
{
    uint32_t crc = 0U;
    for (int i = 0; i < length; i++)
    {
        CRC->DR = pBuffer[i];
    }
    crc = CRC->DR;

    // equivalent C code (with reset to 0xFFFFFFFF at the beginning)
    // uint32_t poly = 0x04C11DB7;
    // uint32_t crcVerify = 0xFFFFFFFF;
    // for (int i = 0; i < length; i++)
    // {
    //     crcVerify ^= pBuffer[i];
    //     for (int j = 0; j < 32; j++)
    //     {
    //         if (crcVerify & 0x80000000)
    //         {
    //             crcVerify = (crcVerify << 1) ^ poly;
    //         }
    //         else
    //         {
    //             crcVerify = (crcVerify << 1);
    //         }
    //     }
    // }

    return crc;

    // Pat Riley GM MK-1201 -00 HW CRC32 calculation = FA1AC95B
    // Pat Riley CRC on first 512 bytes = E6CF6C16
    // Uses CRC-32 (Ethernet) polynomial: 0x4C11DB7
}