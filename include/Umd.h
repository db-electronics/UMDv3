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
        enum UxUserInputState : uint8_t{
            INIT_MAIN_MENU,
            WAIT_FOR_INPUT,
            WAIT_FOR_RELEASE
        };

        enum UxState : uint8_t{
            INIT,
            SELECT_MODE,
            SELECT_MEMORY
        };

        UxState State = INIT;
        UxUserInputState UserInputState = INIT_MAIN_MENU;

        Controls UserInput;

        UMDDisplay Display;
        void UpdateDisplayPathAddressBar(const char* system, const char *path){
            Display.clearLine(0 ,0);
            Display.printf(0, 0, F("UMDv3/%s/%s"), system, path);
        }
        // void PrintOperationTime(){
        //     Display.printf(1, 0, F("%d ms"), Umd::OperationTime);
        // }
    };

    namespace Cart{
        std::unique_ptr<Cartridge> pCartridge;
        Mcp23008 IoExpander;
        std::vector<const char *> MemoryNames;
        std::vector<const char *> Metadata;
        CartridgeFactory Factory;
        Cartridge::CartridgeState State = Cartridge::CartridgeState::IDLE;
        CartridgeActionResult Result;

        class BatchSizeCalculator{
            public:
                BatchSizeCalculator(){};

                void Init(uint32_t totalBytes, uint16_t batchSize){
                    mTotalBytes = totalBytes;
                    mBytesLeft = totalBytes;
                    mMBatchSize = batchSize;
                }

                uint16_t TotalBytes(){
                    return mTotalBytes;
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
                uint32_t mTotalBytes;
                uint32_t mBytesLeft;
                uint32_t mMBatchSize;
        } BatchSizeCalc;
    }

    Mcp23008 IoExpander;
    uint32_t OperationTime;

    const uint16_t BUFFER_SIZE_BYTES = 512;
    /// @brief Data buffer for the UMD, must be a multiple of 4 bytes
    union DataBuffer{
        uint8_t bytes[BUFFER_SIZE_BYTES];
        uint16_t words[BUFFER_SIZE_BYTES/2];
        uint32_t dwords[BUFFER_SIZE_BYTES/4];
    }DataBuffer;
}
