
#include "services/Debouncer.h"

umd::Debouncer::Debouncer()
{
    KeyStates.push_back(KeyState(&Up, UMD_UP_PUSHBUTTON));
    KeyStates.push_back(KeyState(&Down, UMD_DOWN_PUSHBUTTON));
    KeyStates.push_back(KeyState(&Left, UMD_LEFT_PUSHBUTTON));
    KeyStates.push_back(KeyState(&Right, UMD_RIGHT_PUSHBUTTON));
    KeyStates.push_back(KeyState(&Ok, UMD_OK_PUSHBUTTON));
    KeyStates.push_back(KeyState(&Back, UMD_BACK_PUSHBUTTON));
}

void umd::Debouncer::Process(uint8_t inputs, uint32_t currentTicks)
{
    // for (auto& btn : KeyStates)
    // {
    //     SetButtonState(inputs, currentTicks, btn);
    // }
    std::for_each(KeyStates.begin(), KeyStates.end(), [&](auto& btn) {
        SetButtonState(inputs, currentTicks, btn);
    });
}

void umd::Debouncer::SetButtonState(uint8_t inputs, uint32_t currentTicks, KeyState& keyState)
{
    bool isPressed = (inputs & keyState.PinMask) == 0;

    switch (*(keyState.pKeyState))
    {
        case Key::Off:
            *(keyState.pKeyState) = isPressed ? Key::Pressed : Key::Off;
            keyState.PreviousTicks = currentTicks;
            break;
        case Key::Pressed:
            if (isPressed)
            {
                if (currentTicks > (keyState.PreviousTicks + PRESSED_TO_HELD_MS))
                {
                    *(keyState.pKeyState) = Key::Held;
                }
            }
            else
            {
                *(keyState.pKeyState) = Key::Released;
            }
            break;
        case Key::Held:
            *(keyState.pKeyState) = isPressed ? Key::Held : Key::Released;
            keyState.PreviousTicks = currentTicks;
            break;
        case Key::Released:
            if (!isPressed)
            {
                if (currentTicks > (keyState.PreviousTicks + RELEASED_TO_OFF_MS))
                {
                    *(keyState.pKeyState) = Key::Off;
                }
            }
            else
            {
                *(keyState.pKeyState) = Key::Pressed;
                keyState.PreviousTicks = currentTicks;
            }
            break;
        default:
            break;
    }
    
}
