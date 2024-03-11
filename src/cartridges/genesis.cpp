#include "cartridges/genesis.h"

Genesis::Genesis(){
    initIO();
}

Genesis::~Genesis(){}

void Genesis::initIO(){
    setDefaults();

    // set pins to high
    setTIME();
    setAS();
    setLWR();
    setCE();

    setRD();
    setWR();

    dataSetToInputs(true);

    // DTACK on IO0
    ioSetToOutput(0, true);
    setDTACK();

    // VRES on IO1
    ioSetToOutput(1, true);
    setVRES();

    // M3 on IO2
    ioSetToInput(2, true);

    // ASEL on IO3
    ioSetToOutput(3, true);
    setASEL();

    // MRES on IO8
    ioSetToInput(8, false);
}

const char* Genesis::getSystemName(){
    return "Genesis";
}

bool Genesis::readHeader(){
    for(int i = 0; i < GENESIS_HEADER_SIZE; i+=2){
        _header.words[i>>1] = UMD_SWAP_BYTES_16(readWord(GENESIS_HEADER_START_ADDR + i));
    }

    // the numeric values need additional work
    _header.ROMStart = UMD_SWAP_BYTES_32(_header.ROMStart);
    _header.ROMEnd = UMD_SWAP_BYTES_32(_header.ROMEnd);
    _header.RAMStart = UMD_SWAP_BYTES_32(_header.RAMStart);
    _header.RAMEnd = UMD_SWAP_BYTES_32(_header.RAMEnd);
    _header.SRAMStart = UMD_SWAP_BYTES_32(_header.SRAMStart);
    _header.SRAMEnd = UMD_SWAP_BYTES_32(_header.SRAMEnd);
    _header.Checksum = UMD_SWAP_BYTES_16(_header.Checksum);

    ExpectedChecksum = _header.Checksum;

    // check if the first 4 character of _header.SystemType are "SEGA"
    if(_header.SystemType[0] != 'S' || _header.SystemType[1] != 'E' || _header.SystemType[2] != 'G' || _header.SystemType[3] != 'A'){
        return false;
    }

    return true;
}

bool Genesis::calculateChecksum(uint32_t start, uint32_t end){
    uint16_t checksum = 0;
    for(uint32_t i = start; i < end; i+=2){
        checksum += readWord(i);
    }
    
    ActualChecksum = checksum;

    if(checksum == _header.Checksum){
        return true;
    }else{
        return false;
    }
}

FlashInfo Genesis::getFlashInfo(){
    FlashInfo info;

    writeWord(0x00000555 << 1, 0x00AA);
    writeWord(0x000002AA << 1, 0x0055);
    writeWord(0x00000555 << 1, 0x0090);
    info.Manufacturer = readWord(0x00000000);
    info.Device = readWord(0x00000002);
    writeWord(0x00000000, 0x00F0);
    info.Size = getFlashSizeFromInfo(info);
    return info;
}

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

int Genesis::doAction(uint16_t menuIndex, uint16_t menuItemIndex, const SDClass& sd, UMDDisplay& disp)
{
    bool validRom, validChecksum;
    
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
                    uint16_t word;
                    uint32_t romSize;
                    validRom = readHeader();
                    romSize = _header.ROMEnd + 1;
                    validChecksum = calculateChecksum(0x200, romSize);
                    return 0; // index of Main menu
                case 1: // RAM
                    return 0; // index of Main menu
                case 2: // Header
                    validRom = readHeader();
                    disp.printf(1, 5, "S/N: %s", _header.SerialNumber);
                    disp.printf(1, 6, "%s", _header.Copyright);
                    return -1; // stay in Read menu
                default:
                    return 0;
            }
            break;
        case 2: // Write Menu
            switch(menuItemIndex)
            {
                case 0: // ROM
                    _flashInfo = getFlashInfo();
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
                    uint32_t romSize;
                    validRom = readHeader();
                    romSize = _header.ROMEnd + 1;
                    validChecksum = calculateChecksum(0x200, romSize);
                    // show results
                    disp.printf(1, 5, "Expected: %04X", ExpectedChecksum);
                    disp.printf(1, 6, "Actual:   %04X", ActualChecksum);
                    return -1; // stay in Checksum menu
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
    clearCE();
    clearRD();
    wait200ns();
    result = dataReadHigh();
    setRD();
    setCE();
    return result;
}

void Genesis::writeByte(uint16_t address, uint8_t data){
    addressWrite(address);
    dataSetToOutputs();
    dataWriteHigh(data);
    clearCE();
    clearAS();
    clearWR();
    wait200ns();
    setCE();
    setAS();
    setWR();

    // always leave on inputs by default
    dataSetToInputs(true);
}

uint16_t Genesis::readWord(uint32_t address){

    uint16_t result;
    addressWrite(address);
    clearCE();
    clearAS();
    clearRD();
    wait200ns();
    result = dataReadWord();
    setRD();
    setAS();
    setCE();
    return result;
}

void Genesis::writeWord(uint32_t address, uint16_t data){
    addressWrite(address);
    dataSetToOutputs();
    dataWrite(data);
    clearCE();
    clearAS();
    clearWR();
    wait200ns();
    setCE();
    setAS();
    setWR();

    // always leave on inputs by default
    dataSetToInputs(true);
}