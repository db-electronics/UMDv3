
#include "Controls.h"
#include "umdBoardDefinitions.h"

Controls::Controls(){
    this->Up = OFF;
    this->Down = OFF;
    this->Left = OFF;
    this->Right = OFF;
    this->Ok = OFF;
    this->Back = OFF;
}

void Controls::process(uint8_t inputs, uint32_t ticks){

    this->_ticks = ticks;
    this->_setButtonState(&Up, &_upTicks, (UMD_UP_PUSHBUTTON & inputs) == 0);
}

void Controls::_setButtonState(ButtonState *btn, uint32_t *ticks, bool isPressed){
    switch(*btn){
        case OFF:
            *ticks = this->_ticks;
            *btn = isPressed ? PRESSED : OFF;
            break;
        case PRESSED:
            
            break;
    }
}
