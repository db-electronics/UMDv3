#include <Arduino.h>

class Controls{
    public:
        Controls();

        enum ButtonState{
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

        void process(uint8_t inputs, uint32_t ticks);
    
    private:
        void _setButtonState(ButtonState *btn, uint32_t *ticks, bool isPressed);

        uint32_t _ticks;
        uint32_t _upTicks;
        uint32_t _downTicks;
        uint32_t _leftTicks;
        uint32_t _rightTicks;
        uint32_t _okTicks;
        uint32_t _backTicks;

        const uint32_t _pressedToHeldTicks = 500;
        const uint32_t _releasedToOffTicks = 200;


};