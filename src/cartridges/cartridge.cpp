#include "cartridges/cartridge.h"

Cartridge::Cartridge(){
    this->setDefaults();
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
    this->addressWrite(address);
    this->clearCE0();
    this->clearRD();
    wait250ns();
    result = this->dataReadLow();
    this->setRD();
    this->setCE0();
    return result;
}

uint8_t Cartridge::readByte(uint32_t address){

    uint8_t result;
    this->addressWrite(address);
    this->clearCE0();
    this->clearRD();
    wait250ns();
    result = this->dataReadLow();
    this->setRD();
    this->setCE0();
    return result;
}

void Cartridge::writeByte(uint16_t address, uint8_t data){
    this->addressWrite(address);
    this->dataSetToOutputs();
    this->dataWriteLow(data);
    this->clearCE0();
    this->clearWR();
    wait250ns();
    this->setCE0();
    this->setWR();

    // always leave on inputs by default
    this->dataSetToInputs(true);
}

uint16_t Cartridge::readWord(uint32_t address){

    uint16_t result;
    this->addressWrite(address);
    this->clearCE0();
    this->clearRD();
    wait250ns();
    result = this->dataReadWord();
    this->setRD();
    this->setCE0();
    return result;
}