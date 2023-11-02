
#include "Controls.h"


Controls::Controls(){
    this->Up = OFF;
    this->Down = OFF;
    this->Left = OFF;
    this->Right = OFF;
    this->Ok = OFF;
    this->Back = OFF;

    this->_btnStates.push_back(BtnState(&Up, UMD_UP_PUSHBUTTON));
    this->_btnStates.push_back(BtnState(&Down, UMD_DOWN_PUSHBUTTON));
    this->_btnStates.push_back(BtnState(&Left, UMD_LEFT_PUSHBUTTON));
    this->_btnStates.push_back(BtnState(&Right, UMD_RIGHT_PUSHBUTTON));
    this->_btnStates.push_back(BtnState(&Ok, UMD_OK_PUSHBUTTON));
    this->_btnStates.push_back(BtnState(&Back, UMD_BACK_PUSHBUTTON));
}

void Controls::process(uint8_t inputs, uint32_t currentTicks){

    for(auto btn : this->_btnStates){
        this->_setButtonState(inputs, currentTicks, &btn);
    }
}

void Controls::_setButtonState(uint8_t inputs, uint32_t currentTicks, BtnState *btnState){

    bool isPressed = (inputs & btnState->pinMask) == 0;
    
    switch(*(btnState->state)){
        case OFF:
            *(btnState->state) = isPressed ? PRESSED : OFF;
            break;
        case PRESSED:
            if(isPressed){
                if(currentTicks > (btnState->previousTicks + this->_pressedToHeldTicks)){
                    *(btnState->state) = HELD;
                }
            }else{
                *(btnState->state) = RELEASED;
            }
            break;
        case HELD:
            *(btnState->state) = isPressed ? HELD : RELEASED;
            break;
        case RELEASED:
            if(!isPressed){
                if(currentTicks > (btnState->previousTicks + this->_releasedToOffTicks)){
                    *(btnState->state) = OFF;
                }
            }else{
                *(btnState->state) = PRESSED;
            }
            break;
        default:
            break;
    }
}
