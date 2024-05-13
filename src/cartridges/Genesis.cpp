#include "cartridges/Genesis/Genesis.h"

// MARK: Constructor
cartridges::genesis::Cart::Cart(IChecksumCalculator& checksumCalculator)
    : Cartridge(checksumCalculator) {

    InitIO();

    // display will show these memory names in order
    // so here we store an index to the memory enum
    // mMemoryTypeIndexMap[0] = MemoryType::PRG0;
    // mMemoryNames.push_back("ROM");

    // mMemoryTypeIndexMap[1] = MemoryType::RAM0;
    // mMemoryNames.push_back("Save RAM");

    // mMemoryTypeIndexMap[2] = MemoryType::BRAM;
    // mMemoryNames.push_back("SCD Backup RAM");

    mMetadata.clear();
}

// MARK: Destructor
cartridges::genesis::Cart::~Cart(){}

// MARK: InitIO()
void cartridges::genesis::Cart::InitIO(){
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

// MARK: GetCartridgeName()
const char* cartridges::genesis::Cart::GetCartridgeName(){
    ReadHeader();
    return mHeader.Printable.DomesticName;
}

uint32_t cartridges::genesis::Cart::GetCartridgeSize(){
    return GetMemorySize(0);
}

uint32_t cartridges::genesis::Cart::GetMemorySize(uint8_t memTypeIndex){
    // check if the memTypeIndex is valid
    if(!IsMemoryIndexValid(memTypeIndex)){
        return 0;
    }

    MemoryType mem = mMemoryTypeIndexMap[memTypeIndex];

    switch(mem){
        case MemoryType::PRG0:
            ReadHeader();
            return mHeader.ROMEnd + 1;
        default:
            return 0;
    }
}

std::string cartridges::genesis::Cart::GetGameUniqueId() {
    std::stringstream ss;
    ss << std::hex << GetAccumulatedChecksum();
    return ss.str();
}

cartridges::FlashInfo cartridges::genesis::Cart::GetFlashInfo(uint8_t memTypeIndex){

    // check if the memTypeIndex is valid
    if(!IsMemoryIndexValid(memTypeIndex)){
        return FlashInfo(0, 0);
    }

    uint16_t manufacturer, device;
    MemoryType mem = mMemoryTypeIndexMap[memTypeIndex];
    
    switch(mem){
        case MemoryType::PRG0:
            WritePrgWord(0x00000555 << 1, 0xAA00);
            WritePrgWord(0x000002AA << 1, 0x5500);
            WritePrgWord(0x00000555 << 1, 0x9000);
            manufacturer = UMD_SWAP_BYTES_16(ReadPrgWord(0x00000000));
            device = UMD_SWAP_BYTES_16(ReadPrgWord(0x00000002));
            WritePrgWord(0x00000000, 0xF000);
            break;
        default:
            return FlashInfo(0, 0);
    }
    
    FlashInfo info(manufacturer, device);
    return info;
}

// MARK: Identify()
uint32_t cartridges::genesis::Cart::Identify(uint32_t address, cartridges::Array& array, ReadOptions opt){

    array.Next();

    for(int i = 0; i < array.AvailableSize(); i+=2){
        array.Word(i) = ReadPrgWord(address);
        address += 2;
    }

    switch(opt){
        case CHECKSUM_CALCULATOR:
            return mChecksumCalculator.Accumulate(&array.Long(0), array.AvailableSize()/4);
        default:
            return 0;
    }
}

// MARK: ReadMemory()
uint32_t cartridges::genesis::Cart::ReadMemory(uint32_t address, cartridges::Array& array, uint8_t memTypeIndex, ReadOptions opt){
    
    array.Next();

    // check if the memTypeIndex is valid
    if(!IsMemoryIndexValid(memTypeIndex)){
        return 0;
    }

    MemoryType mem = mMemoryTypeIndexMap[memTypeIndex];

    switch(mem){
        case MemoryType::PRG0:
            for(int i = 0; i < array.AvailableSize(); i+=2){
                array.Word(i) = ReadPrgWord(address);
                address += 2;
            }
            break;
        default:
            break;
    }
    switch(opt){
        case CHECKSUM_CALCULATOR:
            return mChecksumCalculator.Accumulate(&array.Long(0), array.AvailableSize()/4);
        default:
            return 0;
    }
}

// MARK: EraseFlash()
int cartridges::genesis::Cart::EraseFlash(uint8_t memTypeIndex){
    // check if the memTypeIndex is valid
    if(!IsMemoryIndexValid(memTypeIndex)){
        return 0;
    }

    MemoryType mem = mMemoryTypeIndexMap[memTypeIndex];
    
    switch(mem){
        case MemoryType::PRG0:
            WritePrgWord(0x00000555 < 1, 0xAA00);
            WritePrgWord(0x000002AA < 1, 0x5500);
            WritePrgWord(0x00000555 < 1, 0x8000);
            WritePrgWord(0x00000555 < 1, 0xAA00);
            WritePrgWord(0x000002AA < 1, 0x5500);
            WritePrgWord(0x00000555 < 1, 0x1000);
            break;
        default:
            return -1;
    }
    return 0;
}

int cartridges::genesis::Cart::ProgramFlash(uint32_t address, uint8_t *buffer, uint16_t size, uint8_t mem){
    return 0;
}

bool cartridges::genesis::Cart::IsFlashBusy(uint8_t mem){
    if((TogglePrgBit(4) != 4)) 
        return false;
    
    return true;
}

// MARK: ReadHeader
void cartridges::genesis::Cart::ReadHeader(){

    for(int i = 0; i < HEADER_SIZE; i+=2){
        mHeader.words[i>>1] = ReadPrgWord(HEADER_START_ADDR + i);
    }

    // the numeric values need additional work
    mHeader.ROMStart = UMD_SWAP_BYTES_32(mHeader.ROMStart);
    mHeader.ROMEnd = UMD_SWAP_BYTES_32(mHeader.ROMEnd);
    mHeader.RAMStart = UMD_SWAP_BYTES_32(mHeader.RAMStart);
    mHeader.RAMEnd = UMD_SWAP_BYTES_32(mHeader.RAMEnd);
    mHeader.SRAMStart = UMD_SWAP_BYTES_32(mHeader.SRAMStart);
    mHeader.SRAMEnd = UMD_SWAP_BYTES_32(mHeader.SRAMEnd);
    mHeader.Checksum = UMD_SWAP_BYTES_16(mHeader.Checksum);

    // ExpectedChecksum = mHeader.Checksum;

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
}

uint8_t cartridges::genesis::Cart::TogglePrgBit(uint8_t attempts){
    uint8_t retValue = 0;
    uint8_t readValue;
    uint8_t oldValue;
    uint8_t i;

    //first read should always be a 1 according to datasheet
    oldValue = ReadPrgWord(0x00000000);

    for(int i = 0; i < attempts; i++){
        readValue = ReadPrgWord(0x00000000) & 0x4000;
        if(oldValue == readValue){
			retValue += 1;
		}else{
			retValue = 0;
		}
		oldValue = readValue;
	}
    
    return retValue;
}

bool cartridges::genesis::Cart::calculateChecksum(uint32_t start, uint32_t end){
    uint16_t checksum = 0;
    for(uint32_t i = start; i < end; i+=2){
        checksum += UMD_SWAP_BYTES_16(ReadPrgWord(i));
    }
    // TODO something meaningful here
    return true;
    // ActualChecksum = checksum;

    // if(checksum == mHeader.Checksum){
    //     return true;
    // }else{
    //     return false;
    // }
}

// int Genesis::doAction(uint16_t menuIndex, uint16_t menuItemIndex, const SDClass& sd, UMDDisplay& disp)
// {
//     bool validRom, validChecksum;
//     String romName;
//     const char* romPath;
//     uint16_t data;
//     uint32_t romSize;

//     switch(menuIndex)
//     {
//         case 0: // Main menu
//             switch(menuItemIndex)
//             {
//                 case 0: // READ
//                     return 1; // index of READ menu
//                 case 1: // Write
//                     return 2;
//                 case 2: // Checksum
//                     return 3;
//                 default: 
//                     return 0;
//             }
//             break;
//         case 1: // Read Menu
//             switch(menuItemIndex)
//             {
//                 case 0: // ROM

//                     ReadHeader();
//                     romSize = mHeader.ROMEnd + 1;
//                     validChecksum = calculateChecksum(0x200, romSize);

//                     romName = "/UMD/Genesis/" + String(mHeader.Printable.SerialNumber) + ".bin";
//                     romPath = romName.c_str();
//                     romFile = sd.open(romPath, FILE_WRITE);

//                     // romFile = sd.open("/UMD/Genesis/rom.bin", FILE_WRITE);

//                     if(!romFile){
//                         //disp.printf(1, 6, "Error opening file");
//                         return -1; // stay in Read menu
//                     }

//                     for(int i = 0; i < romSize; i+=2){
                        
//                         // one by one works
//                         data = ReadPrgWord(i);
//                         romFile.write((uint8_t*)&data, 2);
                        
//                         // this times out
//                         // readPrgWords(i, _dataBuffer.word, 256);
//                         // romFile.write(_dataBuffer.byte, 512);

//                         // mix it up, this also fails...
//                         // readPrgWords(i, _dataBuffer.word, 256);
//                         // for(int j = 0; j < 256; j++){
//                         //     romFile.write((uint8_t*)&_dataBuffer.word[j], 2);
//                         // }
//                         // romFile.flush();
//                     }

//                     romFile.close();

//                     return 0; // index of Main menu
//                 case 1: // RAM
//                     // test the SRAM latch
//                     enableSram(true);
//                     // read from SRAM range to test CE
//                     data = ReadPrgWord(0x200000);
//                     enableSram(false);
//                     // read again, should be from ROM now
//                     data = ReadPrgWord(0x200000);
//                     return 0; // index of Main menu
//                 case 2: // Header
//                     return -1; // stay in Read menu
//                 default:
//                     return 0;
//             }
//             break;
//         case 2: // Write Menu
//             switch(menuItemIndex)
//             {
//                 case 0: // ROM
//                     return 0; // index of Main menu
//                 case 1: // RAM
//                     return 0; // index of Main menu
//                 default:
//                     return 0;
//             }
//             break;
//         case 3: // Checksum Menu
//             switch(menuItemIndex)
//             {
//                 case 0: // Verify Checksum
//                     uint32_t romSize;
//                     ReadHeader();
//                     romSize = mHeader.ROMEnd + 1;
//                     validChecksum = calculateChecksum(0x200, romSize);
//                     // show results
//                     //disp.printf(1, 5, "Expected: %04X", ExpectedChecksum);
//                     //disp.printf(1, 6, "Actual:   %04X", ActualChecksum);
//                     return -1; // stay in Checksum menu
//                 default:
//                     return 0;
//             }
//             break;
//         default:
//             return 0;
//             break;
//     }
// }

void cartridges::genesis::Cart::enableSram(bool enable){

    addressWrite(TIME_CONFIG_ADDR);
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

uint8_t cartridges::genesis::Cart::readPrgByte(uint32_t address){

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

void cartridges::genesis::Cart::writePrgByte(uint32_t address, uint8_t data){
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

uint16_t cartridges::genesis::Cart::ReadPrgWord(uint32_t address){
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

void cartridges::genesis::Cart::WritePrgWord(uint32_t address, uint16_t data){
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

