#pragma once

#include <vector>
#include <memory>
#include <cstdint>
#include <sstream>
#include "cartridges/Cartridge.h"
#include "cartridges/Array.h"
#include "cartridges/Factory.h"
#include "services/Debouncer.h"
#include "services/UmdDisplay.h"
#include "services/Mcp23008.h"
#include "services/IGameIdentifier.h"
#include "services/SdFileGameIdentifier.h"

namespace umd
{
    std::stringstream StringStream;
    std::unique_ptr<IGameIdentifier> pGameIdentifier = std::make_unique<umd::SdFileGameIdentifier>();

    i2cdevice::Mcp23008 IoExpander;
    uint32_t OperationStartTime;
    uint32_t OperationTotalTime;

    cartridges::Array CartridgeData;

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

        // this enum needs to match exactly with the order of MAIN_MENU_ITEMS
        enum CartState : int8_t{
            IDLE = -1,
            IDENTIFY,
            READ,
            WRITE
        };

        std::unique_ptr<cartridges::Cartridge> pCartridge;
        cartridges::Factory Factory;

        CartState State = CartState::IDLE;
        std::string Name = "";
        bool IsIdentified = false;
        
        i2cdevice::Mcp23008 IoExpander;
        std::vector<const char *> MemoryNames;
        std::vector<const char *> Metadata;
        
        bool Identify(bool updateUi);
        bool DumpToFile(uint8_t memTypeIndex, const std::string& filename, bool updateUi);

    }
}

bool umd::Cart::DumpToFile(uint8_t memTypeIndex, const std::string& filename, bool updateUi = false){
    uint32_t currentTicks;
    uint32_t totalBytes;
    uint32_t startTicks;

    currentTicks = HAL_GetTick();
    startTicks = currentTicks;
    totalBytes = pCartridge->GetMemorySize(memTypeIndex);
    CartridgeData.SetTransferSize(totalBytes);

    if(updateUi){
        umd::Ux::Display.SetProgressBarVisibility(true);
    }

    for(int addr = 0; addr < totalBytes; addr += CartridgeData.Size())
    {
        umd::Cart::pCartridge->ReadMemory(addr, CartridgeData, memTypeIndex, cartridges::Cartridge::ReadOptions::NONE);
        if(updateUi && (HAL_GetTick() > currentTicks + umd::Config::PROGRESS_REFRESH_RATE_MS))
        {
            currentTicks = HAL_GetTick();
            umd::Ux::Display.UpdateProgressBar(addr, totalBytes);
            umd::Ux::Display.Redraw();
        }
    }

    return true;
}


bool umd::Cart::Identify(bool updateUi = false){
    uint32_t currentTicks;
    uint32_t totalBytes;
    uint32_t startTicks;
    std::stringstream ss;

    currentTicks = HAL_GetTick();
    startTicks = currentTicks;
    pCartridge->ResetChecksumCalculator();
    totalBytes = pCartridge->GetCartridgeSize();
    CartridgeData.SetTransferSize(totalBytes);

    if(updateUi){
        umd::Ux::Display.SetProgressBarVisibility(true);
    }
    
    for(int addr = 0; addr < totalBytes; addr += CartridgeData.Size())
    {
        umd::Cart::pCartridge->Identify(addr, CartridgeData, cartridges::Cartridge::ReadOptions::CHECKSUM_CALCULATOR);
        if(updateUi && (HAL_GetTick() > currentTicks + umd::Config::PROGRESS_REFRESH_RATE_MS))
        {
            currentTicks = HAL_GetTick();
            umd::Ux::Display.UpdateProgressBar(addr, totalBytes);
            umd::Ux::Display.Redraw();
        }
    }

    OperationTotalTime = HAL_GetTick() - startTicks;
    if(updateUi){
        umd::Ux::Display.SetProgressBarComplete(OperationTotalTime);
    }
    
    ss << std::hex << umd::Cart::pCartridge->GetAccumulatedChecksum();
    if(pGameIdentifier->GameExists(ss.str())){
        umd::Cart::IsIdentified = true;
        umd::Cart::Name = pGameIdentifier->GetGameName(ss.str());
        return true;
    }else{
        umd::Cart::IsIdentified = false;
        umd::Cart::Name = "";
        return false;
    }

    return false;
}