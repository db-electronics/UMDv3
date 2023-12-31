#ifndef CONTROLS_H
#define CONTROLS_H

#include <Arduino.h>
#include <vector>
#include "umdBoardDefinitions.h"

class Controls{
    public:
        Controls();

        enum ButtonState
        {
            OFF,
            PRESSED,
            HELD,
            RELEASED
        };

        ButtonState Up;
        ButtonState Down;
        ButtonState Left;
        ButtonState Right;
        ButtonState Ok;
        ButtonState Back;

        void process(uint8_t inputs, uint32_t currentTicks);
    
    private:

        class BtnState 
        {
            public:
                //BtnState(const BtnState&) = delete;
                BtnState(ButtonState *state, uint8_t mask) : state(state), pinMask(mask){}
                ButtonState *state;
                uint8_t pinMask;
                uint32_t previousTicks;
            
        };

        void _setButtonState(uint8_t inputs, uint32_t currentTicks, BtnState &btnState);
        std::vector<BtnState> _btnStates;

        const uint32_t _pressedToHeldTicks = 500;
        const uint32_t _releasedToOffTicks = 200;
};

#endif