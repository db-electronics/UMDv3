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

std::tuple<const __FlashStringHelper**, uint16_t> Genesis::getMenu(uint16_t id)
{
    switch(id)
    {
        case 0: 
            return std::make_tuple(_mainMenu.Items, _mainMenu.Size);
        case 1: 
            return std::make_tuple(_readMenu.Items, _readMenu.Size);
        case 2: 
            return std::make_tuple(_writeMenu.Items, _writeMenu.Size);
        case 3: 
            return std::make_tuple(_checksumMenu.Items, _checksumMenu.Size);
        default: 
            return std::make_tuple(_mainMenu.Items, _mainMenu.Size);
    }
}

uint16_t Genesis::doAction(uint16_t menuIndex, uint16_t menuItemIndex, const SDClass& sd)
{
    switch(menuIndex)
    {
        case 0: // Main menu
            switch(menuItemIndex)
            {
                case 0: // READ
                    return 1; // index of READ menu
                case 1: // Write
                    return 2;
                case 2: // Checksum
                    return 3;
                default: 
                    return 0;
            }
            break;
        case 1: // Read Menu
            switch(menuItemIndex)
            {
                case 0: // ROM
                    return 0; // index of Main menu
                case 1: // RAM
                    return 0; // index of Main menu
                default:
                    return 0;
            }
            break;
        case 2: // Write Menu
            switch(menuItemIndex)
            {
                case 0: // ROM
                    return 0; // index of Main menu
                case 1: // RAM
                    return 0; // index of Main menu
                default:
                    return 0;
            }
            break;
        case 3: // Checksum Menu
            switch(menuItemIndex)
            {
                case 0: // Verify Checksum
                    return 0; // index of Main menu
                default:
                    return 0;
            }
            break;
        default:
            return 0;
            break;
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
