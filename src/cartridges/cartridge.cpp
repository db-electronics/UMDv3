#include "cartridges/cartridge.h"

Cartridge::Cartridge(){
    this->setDefaults();
}

Cartridge::~Cartridge(){
}

uint8_t Cartridge::readByte(uint16_t address){

    uint8_t result;
    this->addressWrite(address);
    this->waitNs(addressSetupTime);
    this->clearCE0();
    this->clearRD();
    this->waitNs(readHoldTime);
    result = this->dataReadLow();
    this->setRD();
    this->setCE0();
    return  
    result;
}

uint8_t Cartridge::readByte(uint32_t address){

    uint8_t result;
    this->addressWrite(address);
    this->waitNs(addressSetupTime);
    this->clearCE0();
    this->clearRD();
    this->waitNs(readHoldTime);
    result = this->dataReadLow();
    this->setRD();
    this->setCE0();
    return result;
}

void Cartridge::writeByte(uint16_t address, uint8_t data){
    this->addressWrite(address);
    this->waitNs(addressSetupTime);
    this->dataSetToOutputs();
    this->dataWriteLow(data);
    this->clearCE0();
    this->clearWR();
    this->waitNs(writeHoldTime);
    this->setCE0();
    this->setWR();

    // always leave on inputs by default
    this->dataSetToInputs(true);
}

uint16_t Cartridge::readWord(uint32_t address){

    uint16_t result;
    this->addressWrite(address);
    this->waitNs(addressSetupTime);
    this->clearCE0();
    this->clearRD();
    this->waitNs(readHoldTime);
    result = this->dataReadWord();
    this->setRD();
    this->setCE0();
    return result;
}