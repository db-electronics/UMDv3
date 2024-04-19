
#include "services/Controls.h"

Controls::Controls()
{
    Up = OFF;
    Down = OFF;
    Left = OFF;
    Right = OFF;
    Ok = OFF;
    Back = OFF;

    ButtonStates.push_back(BtnState(&Up, UMD_UP_PUSHBUTTON));
    ButtonStates.push_back(BtnState(&Down, UMD_DOWN_PUSHBUTTON));
    ButtonStates.push_back(BtnState(&Left, UMD_LEFT_PUSHBUTTON));
    ButtonStates.push_back(BtnState(&Right, UMD_RIGHT_PUSHBUTTON));
    ButtonStates.push_back(BtnState(&Ok, UMD_OK_PUSHBUTTON));
    ButtonStates.push_back(BtnState(&Back, UMD_BACK_PUSHBUTTON));
}

void Controls::Process(uint8_t inputs, uint32_t currentTicks)
{

    for (auto& btn : ButtonStates)
    {
        SetButtonState(inputs, currentTicks, btn);
    }
}

void Controls::SetButtonState(uint8_t inputs, uint32_t currentTicks, BtnState& btnState)
{
    bool isPressed = (inputs & btnState.PinMask) == 0;

    switch (*(btnState.pButtonstate))
    {
        case OFF:
            *(btnState.pButtonstate) = isPressed ? PRESSED : OFF;
            btnState.PreviousTicks = currentTicks;
            break;
        case PRESSED:
            if (isPressed)
            {
                if (currentTicks > (btnState.PreviousTicks + PRESSED_TO_HELD_MS))
                {
                    *(btnState.pButtonstate) = HELD;
                }
            }
            else
            {
                *(btnState.pButtonstate) = RELEASED;
            }
            break;
        case HELD:
            *(btnState.pButtonstate) = isPressed ? HELD : RELEASED;
            btnState.PreviousTicks = currentTicks;
            break;
        case RELEASED:
            if (!isPressed)
            {
                if (currentTicks > (btnState.PreviousTicks + RELEASED_TO_OFF_MS))
                {
                    *(btnState.pButtonstate) = OFF;
                }
            }
            else
            {
                *(btnState.pButtonstate) = PRESSED;
                btnState.PreviousTicks = currentTicks;
            }
            break;
        default:
            break;
    }
    
}
