#pragma once

#include <vector>
#include <memory>
#include "cartridges/Cartridge.h"
#include "services/CartridgeFactory.h"
#include "services/Controls.h"
#include "services/UmdDisplay.h"
#include "services/Mcp23008.h"

namespace Umd
{
    
    namespace Ux{
        enum UxState
        {
            INIT_MAIN_MENU,
            WAIT_FOR_INPUT,
            WAIT_FOR_RELEASE
        };

        UxState State = INIT_MAIN_MENU;
        UxState NextState = INIT_MAIN_MENU;

        Controls UserInput;
    };

    namespace Cart{
        std::unique_ptr<Cartridge> pCartridge;
        Mcp23008 IoExpander;
        std::vector<const char *> MemoryNames;
        std::vector<const char *> Metadata;
        CartridgeFactory Factory;
        Cartridge::CartridgeState State = Cartridge::CartridgeState::IDLE;
    }

    UMDDisplay Display;
    Mcp23008 IoExpander;

    const uint16_t BUFFER_SIZE_BYTES = 512;
    /// @brief Data buffer for the UMD, must be a multiple of 4 bytes
    union DataBuffer{
        uint8_t bytes[BUFFER_SIZE_BYTES];
        uint16_t words[BUFFER_SIZE_BYTES/2];
        uint32_t dwords[BUFFER_SIZE_BYTES/4];
    }DataBuffer;
}