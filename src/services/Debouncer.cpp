
#include "services/Debouncer.h"

Umd::Debouncer::Debouncer()
{
    Up = ButtonState::Off;
    Down = ButtonState::Off;
    Left = ButtonState::Off;
    Right = ButtonState::Off;
    Ok = ButtonState::Off;
    Back = ButtonState::Off;

    ButtonStates.push_back(BtnState(&Up, UMD_UP_PUSHBUTTON));
    ButtonStates.push_back(BtnState(&Down, UMD_DOWN_PUSHBUTTON));
    ButtonStates.push_back(BtnState(&Left, UMD_LEFT_PUSHBUTTON));
    ButtonStates.push_back(BtnState(&Right, UMD_RIGHT_PUSHBUTTON));
    ButtonStates.push_back(BtnState(&Ok, UMD_OK_PUSHBUTTON));
    ButtonStates.push_back(BtnState(&Back, UMD_BACK_PUSHBUTTON));
}

void Umd::Debouncer::Process(uint8_t inputs, uint32_t currentTicks)
{

    for (auto& btn : ButtonStates)
    {
        SetButtonState(inputs, currentTicks, btn);
    }
}

void Umd::Debouncer::SetButtonState(uint8_t inputs, uint32_t currentTicks, BtnState& btnState)
{
    bool isPressed = (inputs & btnState.PinMask) == 0;

    switch (*(btnState.pButtonstate))
    {
        case ButtonState::Off:
            *(btnState.pButtonstate) = isPressed ? ButtonState::Pressed : ButtonState::Off;
            btnState.PreviousTicks = currentTicks;
            break;
        case ButtonState::Pressed:
            if (isPressed)
            {
                if (currentTicks > (btnState.PreviousTicks + PRESSED_TO_HELD_MS))
                {
                    *(btnState.pButtonstate) = ButtonState::Held;
                }
            }
            else
            {
                *(btnState.pButtonstate) = ButtonState::Released;
            }
            break;
        case ButtonState::Held:
            *(btnState.pButtonstate) = isPressed ? ButtonState::Held : ButtonState::Released;
            btnState.PreviousTicks = currentTicks;
            break;
        case ButtonState::Released:
            if (!isPressed)
            {
                if (currentTicks > (btnState.PreviousTicks + RELEASED_TO_OFF_MS))
                {
                    *(btnState.pButtonstate) = ButtonState::Off;
                }
            }
            else
            {
                *(btnState.pButtonstate) = ButtonState::Pressed;
                btnState.PreviousTicks = currentTicks;
            }
            break;
        default:
            break;
    }
    
}
