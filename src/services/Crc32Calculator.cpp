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
    __HAL_CRC_DR_RESET(&hcrc);
}

// copied from HAL lib because this is disabled in platformio libs :(
uint32_t Crc32Calculator::Accumulate(uint32_t pBuffer[], uint32_t length)
{
    uint32_t index;      /* CRC input data buffer index */
    uint32_t temp = 0U;  /* CRC output (read from hcrc->Instance->DR register) */

    /* Change CRC peripheral state */
    hcrc.State = HAL_CRC_STATE_BUSY;

    /* Enter Data to the CRC calculator */
    for (index = 0U; index < length; index++)
    {
        hcrc.Instance->DR = pBuffer[index];
    }
    temp = hcrc.Instance->DR;

    /* Change CRC peripheral state */
    hcrc.State = HAL_CRC_STATE_READY;

    /* Return the CRC computed value */
    return temp;
}