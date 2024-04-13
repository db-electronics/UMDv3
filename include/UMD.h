#pragma once
#include "services/CartridgeFactory.h"
#include "umdDisplay.h"

namespace UMD
{
    enum UxState
    {
        INIT_MAIN_MENU,
        WAIT_FOR_INPUT,
        WAIT_FOR_RELEASE
    };

    CartridgeFactory CartFactory;
    UMDDisplay Display;
    
    const uint16_t BUFFER_SIZE_BYTES = 512;
    /// @brief Data buffer for the UMD, must be a multiple of 4 bytes
    union DataBuffer{
        uint8_t bytes[BUFFER_SIZE_BYTES];
        uint16_t words[BUFFER_SIZE_BYTES/2];
        uint32_t dwords[BUFFER_SIZE_BYTES/4];
    }DataBuffer;
}