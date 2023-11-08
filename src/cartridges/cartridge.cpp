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