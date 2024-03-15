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

    enableSram(false);
}

const char* Genesis::getSystemName(){
    return "Genesis";
}

bool Genesis::readHeader(){
    for(int i = 0; i < GENESIS_HEADER_SIZE; i+=2){
        _header.words[i>>1] = UMD_SWAP_BYTES_16(readPrgWord(GENESIS_HEADER_START_ADDR + i));
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

    memcpy(_header.Printable.SystemType, _header.SystemType, GENESIS_HEADER_SIZE_OF_SYSTEM_TYPE);
    _header.Printable.SystemType[GENESIS_HEADER_SIZE_OF_SYSTEM_TYPE] = '\0';

    memcpy(_header.Printable.Copyright, _header.Copyright, GENESIS_HEADER_SIZE_OF_COPYRIGHT);
    _header.Printable.Copyright[GENESIS_HEADER_SIZE_OF_COPYRIGHT] = '\0';

    memcpy(_header.Printable.DomesticName, _header.DomesticName, GENESIS_HEADER_SIZE_OF_DOMESTIC_NAME);
    _header.Printable.DomesticName[GENESIS_HEADER_SIZE_OF_DOMESTIC_NAME] = '\0';

    memcpy(_header.Printable.InternationalName, _header.InternationalName, GENESIS_HEADER_SIZE_OF_INTERNATIONAL_NAME);
    _header.Printable.InternationalName[GENESIS_HEADER_SIZE_OF_INTERNATIONAL_NAME] = '\0';

    memcpy(_header.Printable.SerialNumber, _header.SerialNumber, GENESIS_HEADER_SIZE_OF_SERIAL_NUMBER);
    _header.Printable.SerialNumber[GENESIS_HEADER_SIZE_OF_SERIAL_NUMBER] = '\0';

    return true;
}

FlashInfo Genesis::getFlashInfo(){
    FlashInfo info;
    writePrgWord(0x00000555 << 1, 0x00AA);
    writePrgWord(0x000002AA << 1, 0x0055);
    writePrgWord(0x00000555 << 1, 0x0090);
    info.Manufacturer = readPrgWord(0x00000000);
    info.Device = readPrgWord(0x00000002);
    writePrgWord(0x00000000, 0x00F0);
    info.Size = getFlashSizeFromInfo(info);
    return info;
}

bool Genesis::calculateChecksum(uint32_t start, uint32_t end){
    uint16_t checksum = 0;
    for(uint32_t i = start; i < end; i+=2){
        checksum += readPrgWord(i);
    }
    
    ActualChecksum = checksum;

    if(checksum == _header.Checksum){
        return true;
    }else{
        return false;
    }
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
                    // test the SRAM latch
                    enableSram(true);
                    enableSram(false);
                    return 0; // index of Main menu
                case 2: // Header
                    validRom = readHeader();
                    if(validRom){
                        disp.printf(1, 0, " %s", _header.Printable.SystemType);
                        disp.printf(1, 1, " %s", _header.Printable.Copyright);
                        disp.printf(1, 2, " %s", _header.Printable.DomesticName);
                        disp.printf(1, 3, " %s", _header.Printable.InternationalName);
                        disp.printf(1, 4, " %s", _header.Printable.SerialNumber);
                        disp.printf(1, 5, "Size: 0x%08X", _header.ROMEnd + 1);
                        disp.printf(1, 6, "CRC : 0x%04X", _header.Checksum);
                    }else{
                        disp.printf(1, 6, "Invalid ROM");
                    }
                    return -1; // stay in Read menu
                case 3: // Flash ID
                    _flashInfo = getFlashInfo();
                    disp.printf(1, 5, "Manufacturer: %04X", _flashInfo.Manufacturer);
                    disp.printf(1, 6, "Device:       %04X", _flashInfo.Device);
                    return -1; // stay in Read menu
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

void Genesis::enableSram(bool enable){

    addressWrite(_timeConfigAddr);
    dataSetToOutputs();

    if(enable){
        // Write 0x01 to 0xA130F1
        dataWriteLow(0x01);

    }else{
        // Write 0x00 to 0xA130F1
        dataWriteLow(0x00);
    }

    clearCE();
    clearTIME();
    clearLWR();
    wait200ns();
    setLWR();
    setTIME();
    setCE();
    // always leave on inputs by default
    dataSetToInputs(true);
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

uint16_t Genesis::readPrgWord(uint32_t address){

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

void Genesis::writePrgWord(uint32_t address, uint16_t data){
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

void Genesis::readWords(uint32_t address, uint16_t *buffer, uint16_t size){

    for(int i = 0; i < size; i++){
        addressWrite(address);
        clearCE();
        clearAS();
        clearRD();
        wait200ns();
        *(buffer++) = dataReadWord();
        setRD();
        setAS();
        setCE();
        address += 2;
    }
}