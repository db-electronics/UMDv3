#pragma once

#include <vector>
#include <memory>
#include "cartridges/Cartridge.h"
#include "services/CartridgeFactory.h"
#include "services/Debouncer.h"
#include "services/UmdDisplay.h"
#include "services/Mcp23008.h"
#include "services/UmdArray.h"
#include "services/BatchSizeCalculator.h"

namespace umd
{
    namespace Config{

        const uint32_t DAS_REPEAT_RATE_MS = 75;
        const uint32_t PROGRESS_REFRESH_RATE_MS = 100;
        const uint16_t BUFFER_SIZE_BYTES = 512;
        const uint8_t MCP23008_BOARD_ADDRESS = 0x27;
        const uint8_t MCP23008_ADAPTER_ADDRESS = 0x20;
    }

    namespace Ux{
        // TODO enum class
        enum UxUserInputState : uint8_t{
            UX_INPUT_INIT,
            UX_INPUT_WAIT_FOR_PRESSED,
            UX_INPUT_WAIT_FOR_RELEASED
        };

        // TODO enum class
        enum UxState : uint8_t{
            UX_MAIN_MENU,
            UX_OPERATION_COMPLETE,
            UX_SELECT_MEMORY
        };

        UxState State = UX_MAIN_MENU;
        UxUserInputState UserInputState = UX_INPUT_INIT;

        umd::Debouncer Keys;

        std::unique_ptr<Adafruit_SSD1306> pSSD1306 = std::make_unique<Adafruit_SSD1306>(OLED_SCREEN_WIDTH, OLED_SCREEN_HEIGHT, &Wire, OLED_RESET);
        UMDDisplay Display = UMDDisplay(std::move(pSSD1306));

        const std::vector<const char *> MAIN_MENU_ITEMS = {
            "Identify",
            "Read",
            "Write",
            "Flash"
        };

        const std::vector<const char *> MENU_WITH_30_ITEMS = {
            "0", 
            "this is a bit longer string", 
            "this item should be a pretty long string.", 
            "3", "4", "5", "6", "7", "8", "9",
            "10", "11", "12", "13", "14", "15", "16", "17", "18", "19",
            "20", "21", "22", "23", "24", "25", "26", "27", 
            "RetroRGB fucks meese, I have proof.", 
            "29"
        };
    };

    namespace Cart{
        std::unique_ptr<Cartridge> pCartridge;
        i2cdevice::Mcp23008 IoExpander;
        std::vector<const char *> MemoryNames;
        std::vector<const char *> Metadata;
        CartridgeFactory Factory;
        Cartridge::CartridgeState State = Cartridge::CartridgeState::IDLE;
        CartridgeActionResult Result;        

        void ClearMetadata(){
            for(auto item : Metadata){
                free((void *)item);
            }
            Metadata.clear();
        }

        umd::BatchSizeCalculator BatchSizeCalc;
    }

    i2cdevice::Mcp23008 IoExpander;
    uint32_t OperationStartTime;
    uint32_t OperationTotalTime;

    
    /// @brief Data buffer for the UMD, must be a multiple of 4 bytes
    // union DataBuffer{
    //     uint8_t bytes[BUFFER_SIZE_BYTES];
    //     uint16_t words[BUFFER_SIZE_BYTES/2];
    //     uint32_t dwords[BUFFER_SIZE_BYTES/4];
    // }DataBuffer;
    std::array<uint8_t, umd::Config::BUFFER_SIZE_BYTES> DataBuffer;

    UmdArray<Config::BUFFER_SIZE_BYTES> Array;
}
