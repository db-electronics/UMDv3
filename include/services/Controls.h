#pragma once

#include <Arduino.h>
#include <vector>
#include "config/UMDBoardDefinitions.h"

class Controls{
    public:
        Controls();

        enum ButtonState
        {
            OFF,
            RELEASED,
            PRESSED,
            HELD
        };

        ButtonState Up;
        ButtonState Down;
        ButtonState Left;
        ButtonState Right;
        ButtonState Ok;
        ButtonState Back;

        void Process(uint8_t inputs, uint32_t currentTicks);
    
    private:

        class BtnState 
        {
            public:
                BtnState(ButtonState *state, uint8_t mask) : pButtonstate(state), PinMask(mask){}
                ButtonState *pButtonstate;
                uint8_t PinMask;
                uint32_t PreviousTicks;
            
        };

        void SetButtonState(uint8_t inputs, uint32_t currentTicks, BtnState &btnState);
        std::vector<BtnState> ButtonStates;

        const uint32_t PRESSED_TO_HELD_MS = 250;
        const uint32_t RELEASED_TO_OFF_MS = 200;
};
