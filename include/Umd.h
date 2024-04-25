#pragma once

#include <vector>
#include <memory>
#include "cartridges/Cartridge.h"
#include "cartridges/Array.h"
#include "cartridges/Factory.h"
#include "services/Debouncer.h"
#include "services/UmdDisplay.h"
#include "services/Mcp23008.h"

namespace umd
{
    namespace Config{

        const uint32_t DAS_REPEAT_RATE_MS = 75;
        const uint32_t PROGRESS_REFRESH_RATE_MS = 100;
        const uint8_t MCP23008_BOARD_ADDRESS = 0x27;
        const uint8_t MCP23008_ADAPTER_ADDRESS = 0x20;

        using namespace i2cdevice;
        const uint8_t MCP23008_LEFT_PIN = Mcp23008::Pins::GP0;
        const uint8_t MCP23008_DOWN_PIN = Mcp23008::Pins::GP1;
        const uint8_t MCP23008_UP_PIN = Mcp23008::Pins::GP2;
        const uint8_t MCP23008_RIGHT_PIN = Mcp23008::Pins::GP3;
        const uint8_t MCP23008_OK_PIN = Mcp23008::Pins::GP4;
        const uint8_t MCP23008_BACK_PIN = Mcp23008::Pins::GP5;
        const uint8_t MCP23008_LED_0 = Mcp23008::Pins::GP6;
        const uint8_t MCP23008_LED_1 = Mcp23008::Pins::GP7;
        const uint8_t MCP23008_PUSHBUTTONS = MCP23008_LEFT_PIN | MCP23008_DOWN_PIN | MCP23008_UP_PIN | MCP23008_RIGHT_PIN | MCP23008_OK_PIN | MCP23008_BACK_PIN;
        const uint8_t MCP23008_LEDS = MCP23008_LED_0 | MCP23008_LED_1;
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

        umd::Debouncer Keys = umd::Debouncer(
            Config::MCP23008_LEFT_PIN, 
            Config::MCP23008_DOWN_PIN, 
            Config::MCP23008_UP_PIN, 
            Config::MCP23008_RIGHT_PIN, 
            Config::MCP23008_OK_PIN, 
            Config::MCP23008_BACK_PIN);

        UxState State = UX_MAIN_MENU;
        UxUserInputState UserInputState = UX_INPUT_INIT;

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
        std::unique_ptr<cartridges::Cartridge> pCartridge;
        i2cdevice::Mcp23008 IoExpander;
        std::vector<const char *> MemoryNames;
        std::vector<const char *> Metadata;
        cartridges::Factory Factory;
        cartridges::Cartridge::CartridgeState State = cartridges::Cartridge::CartridgeState::IDLE;     

        // TODO get rid of metadata
        void ClearMetadata(){
            for(auto item : Metadata){
                free((void *)item);
            }
            Metadata.clear();
        }
    }

    i2cdevice::Mcp23008 IoExpander;
    uint32_t OperationStartTime;
    uint32_t OperationTotalTime;

    cartridges::Array CartridgeData;
}
