#include "cartridges/Genesis.h"

// MARK: Constructor
Genesis::Genesis(IChecksumCalculator& checksumCalculator)
    : Cartridge(checksumCalculator) {

    InitIO();

    // display will show these memory names in order
    // so here we store an index to the memory enum
    mMemoryIndexToType[0] = PRG0;
    mMemoryNames.push_back("ROM");

    mMemoryIndexToType[1] = RAM0;
    mMemoryNames.push_back("Save RAM");

    mMemoryIndexToType[2] = BRAM;
    mMemoryNames.push_back("SCD Backup RAM");

    mMetadata.clear();
    mHeader.HasData = false;
}

// MARK: Destructor
Genesis::~Genesis(){}

// MARK: InitIO()
void Genesis::InitIO(){
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

// MARK: GetSystemName()
const char* Genesis::GetSystemName(){
    return "MD";
}

// MARK: GetCartridgeName()
const char* Genesis::GetCartridgeName(){
    ReadHeader();
    return mHeader.Printable.DomesticName;
}

uint32_t Genesis::GetCartridgeSize(){
    ReadHeader();
    return mHeader.ROMEnd + 1;
}

FlashInfo Genesis::GetFlashInfo(MemoryType mem){
    uint16_t manufacturer, device;

    switch(mem){
        case PRG0:
            writePrgWord(0x00000555 << 1, 0xAA00);
            writePrgWord(0x000002AA << 1, 0x5500);
            writePrgWord(0x00000555 << 1, 0x9000);
            manufacturer = UMD_SWAP_BYTES_16(readPrgWord(0x00000000));
            device = UMD_SWAP_BYTES_16(readPrgWord(0x00000002));
            writePrgWord(0x00000000, 0xF000);
            break;
        default:
            return FlashInfo(0, 0);
    }
    
    FlashInfo info(manufacturer, device);
    return info;
}

// MARK: Identify()
uint32_t Genesis::Identify(uint32_t address, uint8_t *buffer, uint16_t size, ReadOptions opt){
    // fill the buffer, but don't modify its value because we may need it for checksum
    for(int i = 0; i < size; i+=2){
        *(uint16_t*)(buffer + i) = readPrgWord(address);
        address += 2;
    }

    switch(opt){
        case HW_CHECKSUM:
            return mChecksumCalculator.Accumulate((uint32_t*)buffer, size/4);
        default:
            return 0;
    }
}

// MARK: ReadMemory()
void Genesis::ReadMemory(uint32_t address, uint8_t *buffer, uint16_t size, MemoryType mem, ReadOptions opt){
    switch(mem){
        case PRG0:
            for(int i = 0; i < size; i+=2){
                *(uint16_t*)(buffer + i) = readPrgWord(address);
                address += 2;
            }
            break;
        default:
            break;
    }
}

// MARK: EraseFlash()
int Genesis::EraseFlash(MemoryType mem){
    switch(mem){
        case PRG0:
            writePrgWord(0x00000555 < 1, 0xAA00);
            writePrgWord(0x000002AA < 1, 0x5500);
            writePrgWord(0x00000555 < 1, 0x8000);
            writePrgWord(0x00000555 < 1, 0xAA00);
            writePrgWord(0x000002AA < 1, 0x5500);
            writePrgWord(0x00000555 < 1, 0x1000);
            break;
        default:
            return -1;
    }
    return 0;
}

int Genesis::flashProgram(uint32_t address, uint8_t *buffer, uint16_t size, uint8_t mem){
    return 0;
}

bool Genesis::flashIsBusy(uint8_t mem){
    if((togglePrgBit(4) != 4)) 
        return false;
    
    return true;
}

// MARK: ReadHeader
void Genesis::ReadHeader(){

    if(mHeader.HasData){
        return;
    }

    for(int i = 0; i < GENESIS_HEADER_SIZE; i+=2){
        mHeader.words[i>>1] = readPrgWord(GENESIS_HEADER_START_ADDR + i);
    }

    // the numeric values need additional work
    mHeader.ROMStart = UMD_SWAP_BYTES_32(mHeader.ROMStart);
    mHeader.ROMEnd = UMD_SWAP_BYTES_32(mHeader.ROMEnd);
    mHeader.RAMStart = UMD_SWAP_BYTES_32(mHeader.RAMStart);
    mHeader.RAMEnd = UMD_SWAP_BYTES_32(mHeader.RAMEnd);
    mHeader.SRAMStart = UMD_SWAP_BYTES_32(mHeader.SRAMStart);
    mHeader.SRAMEnd = UMD_SWAP_BYTES_32(mHeader.SRAMEnd);
    mHeader.Checksum = UMD_SWAP_BYTES_16(mHeader.Checksum);

    ExpectedChecksum = mHeader.Checksum;

    // check if the first 4 character of mHeader.SystemType are "SEGA"
    // if(mHeader.SystemType[0] != 'S' || mHeader.SystemType[1] != 'E' || mHeader.SystemType[2] != 'G' || mHeader.SystemType[3] != 'A'){
    //     return false;
    // }

    // TODO: maybe trim whitespace on the Domestic and International names?
    memcpy(mHeader.Printable.SystemType, mHeader.SystemType, GENESIS_HEADER_SIZE_OF_SYSTEM_TYPE);
    mHeader.Printable.SystemType[GENESIS_HEADER_SIZE_OF_SYSTEM_TYPE] = '\0';

    memcpy(mHeader.Printable.Copyright, mHeader.Copyright, GENESIS_HEADER_SIZE_OF_COPYRIGHT);
    mHeader.Printable.Copyright[GENESIS_HEADER_SIZE_OF_COPYRIGHT] = '\0';

    memcpy(mHeader.Printable.DomesticName, mHeader.DomesticName, GENESIS_HEADER_SIZE_OF_DOMESTIC_NAME);
    mHeader.Printable.DomesticName[GENESIS_HEADER_SIZE_OF_DOMESTIC_NAME] = '\0';

    memcpy(mHeader.Printable.InternationalName, mHeader.InternationalName, GENESIS_HEADER_SIZE_OF_INTERNATIONAL_NAME);
    mHeader.Printable.InternationalName[GENESIS_HEADER_SIZE_OF_INTERNATIONAL_NAME] = '\0';

    memcpy(mHeader.Printable.SerialNumber, mHeader.SerialNumber, GENESIS_HEADER_SIZE_OF_SERIAL_NUMBER);
    mHeader.Printable.SerialNumber[GENESIS_HEADER_SIZE_OF_SERIAL_NUMBER] = '\0';

    mMetadata.clear();
    mMetadata.push_back(mHeader.Printable.DomesticName);
    mMetadata.push_back(mHeader.Printable.SerialNumber);

    mHeader.HasData = true;
}

void Genesis::erasePrgFlash(bool wait){
    writePrgWord(0x00000555 < 1, 0xAA00);
    writePrgWord(0x000002AA < 1, 0x5500);
    writePrgWord(0x00000555 < 1, 0x8000);
    writePrgWord(0x00000555 < 1, 0xAA00);
    writePrgWord(0x000002AA < 1, 0x5500);
    writePrgWord(0x00000555 < 1, 0x1000);
    if(wait){
        while(togglePrgBit(4) != 4);
    }
}

uint8_t Genesis::togglePrgBit(uint8_t attempts){
    uint8_t retValue = 0;
    uint8_t readValue;
    uint8_t oldValue;
    uint8_t i;

    //first read should always be a 1 according to datasheet
    oldValue = readPrgWord(0x00000000);

    for(int i = 0; i < attempts; i++){
        readValue = readPrgWord(0x00000000) & 0x4000;
        if(oldValue == readValue){
			retValue += 1;
		}else{
			retValue = 0;
		}
		oldValue = readValue;
	}
    
    return retValue;
}

bool Genesis::calculateChecksum(uint32_t start, uint32_t end){
    uint16_t checksum = 0;
    for(uint32_t i = start; i < end; i+=2){
        checksum += UMD_SWAP_BYTES_16(readPrgWord(i));
    }
    
    ActualChecksum = checksum;

    if(checksum == mHeader.Checksum){
        return true;
    }else{
        return false;
    }
}

CartridgeActionResult Genesis::act(CartridgeState state, uint16_t menuItemIndex)
{
    switch(menuItemIndex)
    {
        default: 
            return CartridgeActionResult();
    }
} 

int Genesis::doAction(uint16_t menuIndex, uint16_t menuItemIndex, const SDClass& sd, UMDDisplay& disp)
{
    bool validRom, validChecksum;
    String romName;
    const char* romPath;
    uint16_t data;
    uint32_t romSize;

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

                    ReadHeader();
                    romSize = mHeader.ROMEnd + 1;
                    validChecksum = calculateChecksum(0x200, romSize);

                    romName = "/UMD/Genesis/" + String(mHeader.Printable.SerialNumber) + ".bin";
                    romPath = romName.c_str();
                    romFile = sd.open(romPath, FILE_WRITE);

                    // romFile = sd.open("/UMD/Genesis/rom.bin", FILE_WRITE);

                    if(!romFile){
                        disp.printf(1, 6, "Error opening file");
                        return -1; // stay in Read menu
                    }

                    for(int i = 0; i < romSize; i+=2){
                        
                        // one by one works
                        data = readPrgWord(i);
                        romFile.write((uint8_t*)&data, 2);
                        
                        // this times out
                        // readPrgWords(i, _dataBuffer.word, 256);
                        // romFile.write(_dataBuffer.byte, 512);

                        // mix it up, this also fails...
                        // readPrgWords(i, _dataBuffer.word, 256);
                        // for(int j = 0; j < 256; j++){
                        //     romFile.write((uint8_t*)&_dataBuffer.word[j], 2);
                        // }
                        // romFile.flush();
                    }

                    romFile.close();

                    return 0; // index of Main menu
                case 1: // RAM
                    // test the SRAM latch
                    enableSram(true);
                    // read from SRAM range to test CE
                    data = readPrgWord(0x200000);
                    enableSram(false);
                    // read again, should be from ROM now
                    data = readPrgWord(0x200000);
                    return 0; // index of Main menu
                case 2: // Header
                    ReadHeader();
                    if(validRom){
                        disp.printf(1, 0, " %s", mHeader.Printable.SystemType);
                        disp.printf(1, 1, " %s", mHeader.Printable.Copyright);
                        disp.printf(1, 2, " %s", mHeader.Printable.DomesticName);
                        disp.printf(1, 3, " %s", mHeader.Printable.InternationalName);
                        disp.printf(1, 4, " %s", mHeader.Printable.SerialNumber);
                        disp.printf(1, 5, "Size: 0x%06X", mHeader.ROMEnd + 1);
                        disp.printf(1, 6, "CRC : 0x%04X", mHeader.Checksum);
                    }else{
                        disp.printf(1, 6, "Invalid ROM");
                    }
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
                    ReadHeader();
                    romSize = mHeader.ROMEnd + 1;
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

    clearCE();  // TODO remove CE when new cart is ready
    clearTIME();
    clearLWR();
    wait200ns();
    setLWR();
    setTIME();
    setCE();
    // always leave on inputs by default
    dataSetToInputs(true);
}

uint8_t Genesis::readPrgByte(uint32_t address){

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

void Genesis::writePrgByte(uint32_t address, uint8_t data){
    addressWrite(address);
    dataSetToOutputs();
    dataWriteLow(data);
    clearCE();
    clearAS();
    clearWR();
    wait200ns();
    setWR();
    setAS();
    setCE();

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
    result = dataReadWordSwapped();
    setRD();
    setAS();
    setCE();
    return result;
}

void Genesis::writePrgWord(uint32_t address, uint16_t data){
    addressWrite(address);
    dataSetToOutputs();
    dataWriteSwapped(data);
    clearCE();
    clearAS();
    clearWR();
    wait200ns();
    setWR();
    setAS();
    setCE();

    // always leave on inputs by default
    dataSetToInputs(true);
}

void Genesis::readPrgWords(uint32_t address, uint16_t *buffer, uint16_t size){

    for(int i = 0; i < size; i++){
        addressWrite(address);
        clearCE();
        clearAS();
        clearRD();
        wait200ns();
        *(buffer++) = dataReadWordSwapped();
        setRD();
        setAS();
        setCE();
        address += 2;
    }
}
