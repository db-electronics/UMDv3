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
            void Process(uint8_t inputs, uint32_t currentTicks);
        
            Key Up = Key::Off;
            Key Down = Key::Off;
            Key Left = Key::Off;
            Key Right = Key::Off;
            Key Ok = Key::Off;
            Key Back = Key::Off;

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


