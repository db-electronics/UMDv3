#include "cartridges/genesis.h"

Genesis::Genesis(){
    this->setDefaults();
}

Genesis::~Genesis(){}

const char* Genesis::getSystemName(){
    return "Genesis";
}

// const __FlashStringHelper** Genesis::getMenuItems(int id)
// {
//     //return _menuTopLevel;
//     switch(id){
//         case 0: return _mainMenu.Items; break;
//         default: return _mainMenu.Items; break;
//     }
// }

// int Genesis::getMenuSize(int id)
// {
//     switch(id){
//         case 0: return _mainMenu.Size; break;
//         default: return _mainMenu.Size; break;
//     }
// }

std::tuple<const __FlashStringHelper**, int> Genesis::getMenu(int id)
{
    switch(id)
    {
        case 0:
        default: return std::make_tuple(_mainMenu.Items, _mainMenu.Size);
    }
}

uint8_t Genesis::readByte(uint32_t address){

    uint8_t result;
    addressWrite(address);
    clearCE0();
    clearRD();
    wait200ns();
    result = dataReadHigh();
    setRD();
    setCE0();
    return result;
}

void Genesis::writeByte(uint16_t address, uint8_t data){
    addressWrite(address);
    dataSetToOutputs();
    dataWriteHigh(data);
    clearCE0();
    clearWR();
    wait200ns();
    setCE0();
    setWR();

    // always leave on inputs by default
    dataSetToInputs(true);
}

uint16_t Genesis::readWord(uint32_t address){

    uint16_t result;
    addressWrite(address);
    clearCE0();
    clearRD();
    wait200ns();
    result = dataReadWordSwapped();
    setRD();
    setCE0();
    return result;
}
