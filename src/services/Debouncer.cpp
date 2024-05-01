
#include "services/Debouncer.h"

umd::Debouncer::Debouncer(
    uint8_t leftPinMask, 
    uint8_t downPinMask, 
    uint8_t upPinMask, 
    uint8_t rightPinMask, 
    uint8_t okPinMask, 
    uint8_t backPinMask)
{
    KeyStates.push_back(KeyState(&Up, upPinMask));
    KeyStates.push_back(KeyState(&Down, downPinMask));
    KeyStates.push_back(KeyState(&Left, leftPinMask));
    KeyStates.push_back(KeyState(&Right, rightPinMask));
    KeyStates.push_back(KeyState(&Ok, okPinMask));
    KeyStates.push_back(KeyState(&Back, backPinMask));
}

void umd::Debouncer::Process(uint8_t inputs, uint32_t currentTicks)
{
    for (auto& keyState : KeyStates)
    {
        SetButtonState(inputs, currentTicks, keyState);
    }
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
