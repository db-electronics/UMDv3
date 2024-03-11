#include "cartridges/cartridge.h"

Cartridge::Cartridge(){
    setDefaults();
}

Cartridge::~Cartridge(){
}

void Cartridge::testWait(void){
    clearCE0();
    wait100ns();
    setCE0();
    clearCE0();
    wait150ns();
    setCE0();
    clearCE0();
    wait200ns();
    setCE0();
    clearCE0();
    wait250ns();
    setCE0();
}

uint32_t Cartridge::getFlashSizeFromInfo(FlashInfo info){
    
    uint32_t size = 0;

    switch(info.Manufacturer){
        // microchip
        case 0xBF:
            switch(info.Device)
            {
                case 0x6D: // SST39VF6401B
                case 0x6C: // SST39VF6402B
                    size = 0x800000;
                    break;
                case 0x5D: // SST39VF3201B
                case 0x5C: // SST39VF3202B
                case 0x5B: // SST39VF3201
                case 0x5A: // SST39VF3202
                    size = 0x400000;
                    break;
                case 0x4F: // SST39VF1601C
                case 0x4E: // SST39VF1602C
                case 0x4B: // SST39VF1601
                case 0x4A: // SST39VF1602
                    size = 0x200000;
                    break;
                default:
                    break;
            }
            break;
        
        // macronix
        case 0xC2:
            switch(info.Device)
            {
                // chips which will be single per board
                // 3.3V
                case 0xC9: // MX29LV640ET
                case 0xCB: // MX29LV640EB
                    size = 0x800000;
                    break;
                case 0xA7: // MX29LV320ET
                case 0xA8: // MX29LV320EB
                    size = 0x400000;
                    break;
                case 0xC4: // MX29LV160DT
                case 0x49: // MX29LV160DB
                    size = 0x200000;
                    break;    
                // 5V
                case 0x58: // MX29F800CT
                case 0xD6: // MX29F800CB
                    size = 0x100000;
                    break;
                case 0x23: // MX29F400CT
                case 0x2223:
                case 0xAB: // MX29F400CB
                case 0x22AB:
                    size = 0x80000;
                    break;
                case 0x51: // MX29F200CT
                case 0x57: // MX29F200CB
                    size = 0x40000;
                    break;
                default:
                    break;
            }
            break;
            
        default:
            break;
    }

    return size;
}

uint8_t Cartridge::readByte(uint16_t address){

    uint8_t result;
    addressWrite(address);
    clearCE0();
    clearRD();
    wait250ns();
    result = dataReadLow();
    setRD();
    setCE0();
    return result;
}

uint8_t Cartridge::readByte(uint32_t address){

    uint8_t result;
    addressWrite(address);
    clearCE0();
    clearRD();
    wait250ns();
    result = dataReadLow();
    setRD();
    setCE0();
    return result;
}

void Cartridge::readBytes(uint32_t address, uint8_t *buffer, uint16_t size){

    for(int i = 0; i < size; i++){
        addressWrite(address++);
        clearCE0();
        clearRD();
        wait250ns();
        *(buffer++) = dataReadLow();
        setRD();
        setCE0();
    }
}

void Cartridge::writeByte(uint16_t address, uint8_t data){
    addressWrite(address);
    dataSetToOutputs();
    dataWriteLow(data);
    clearCE0();
    clearWR();
    wait250ns();
    setCE0();
    setWR();

    // always leave on inputs by default
    dataSetToInputs(true);
}

uint16_t Cartridge::readWord(uint32_t address){

    uint16_t result;
    addressWrite(address);
    clearCE0();
    clearRD();
    wait250ns();
    result = dataReadWord();
    setRD();
    setCE0();
    return result;
}

void Cartridge::writeWord(uint32_t address, uint16_t data){
    addressWrite(address);
    dataSetToOutputs();
    dataWrite(data);
    clearCE0();
    clearWR();
    wait250ns();
    setCE0();
    setWR();

    // always leave on inputs by default
    dataSetToInputs(true);
}