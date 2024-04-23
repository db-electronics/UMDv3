#pragma once

#include <Arduino.h>
#include <vector>
#include "config/UMDBoardDefinitions.h"

namespace umd {

    enum class Key
    {
        Off,
        Released,
        Pressed,
        Held
    };
    
    class Debouncer{
        public:
            Debouncer();



            Key Up;
            Key Down;
            Key Left;
            Key Right;
            Key Ok;
            Key Back;

            void Process(uint8_t inputs, uint32_t currentTicks);
        
        private:

            class KeyState 
            {
                public:
                    KeyState(Key *state, uint8_t mask) 
                        : pKeyState(state), PinMask(mask){}
                    Key *pKeyState;
                    uint8_t PinMask;
                    uint32_t PreviousTicks;
                
            };

            void SetButtonState(uint8_t inputs, uint32_t currentTicks, KeyState& keyState);
            std::vector<KeyState> KeyStates;

            const uint32_t PRESSED_TO_HELD_MS = 250;
            const uint32_t RELEASED_TO_OFF_MS = 200;
    };
}


