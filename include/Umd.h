#pragma once

#include <vector>
#include <memory>
#include "cartridges/Cartridge.h"
#include "services/CartridgeFactory.h"
#include "services/Controls.h"
#include "services/UmdDisplay.h"
#include "services/Mcp23008.h"

#define UMD_DATA_BUFFER_SIZE_BYTES 512

namespace Umd
{
    
    namespace Ux{
        enum UxUserInputState : uint8_t{
            UX_INPUT_INIT,
            UX_INPUT_WAIT_FOR_PRESSED,
            UX_INPUT_WAIT_FOR_RELEASED
        };

        enum UxState : uint8_t{
            UX_MAIN_MENU,
            UX_OPERATION_COMPLETE,
            UX_SELECT_MEMORY
        };

        UxState State = UX_MAIN_MENU;
        UxUserInputState UserInputState = UX_INPUT_INIT;

        Controls UserInput;

        std::unique_ptr<Adafruit_SSD1306> pSSD1306 = std::make_unique<Adafruit_SSD1306>(OLED_SCREEN_WIDTH, OLED_SCREEN_HEIGHT, &Wire, OLED_RESET);
        UMDDisplay Display = UMDDisplay(std::move(pSSD1306));

        const std::vector<const char *> MAIN_MENU_ITEMS = {
            "Identify",
            "Read",
            "Write"
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
        Mcp23008 IoExpander;
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

        class BatchSizeCalculator{
            public:
                BatchSizeCalculator(){};

                void Init(uint32_t totalBytes, uint16_t batchSize){
                    mBytesLeft = totalBytes;
                    mMBatchSize = batchSize;
                }

                uint16_t Next(){
                    if(mBytesLeft >= mMBatchSize){
                        mBytesLeft -= mMBatchSize;
                        return mMBatchSize;
                    }
                    else{
                        return mBytesLeft;
                    }
                };

            private:
                uint32_t mBytesLeft;
                uint32_t mMBatchSize;
        } BatchSizeCalc;
    }

    Mcp23008 IoExpander;
    uint32_t OperationTime;

    const uint16_t BUFFER_SIZE_BYTES = 512;
    /// @brief Data buffer for the UMD, must be a multiple of 4 bytes
    // union DataBuffer{
    //     uint8_t bytes[BUFFER_SIZE_BYTES];
    //     uint16_t words[BUFFER_SIZE_BYTES/2];
    //     uint32_t dwords[BUFFER_SIZE_BYTES/4];
    // }DataBuffer;
    std::array<uint8_t, UMD_DATA_BUFFER_SIZE_BYTES> DataBuffer;
}
