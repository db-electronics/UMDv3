#include "cartridges/genesis.h"

Genesis::Genesis(){
    this->setDefaults();
}

Genesis::~Genesis(){}

const char* Genesis::getSystemName(){
    return "Genesis";
}

uint8_t Genesis::readByte(uint32_t address){

    uint8_t result;
    this->addressWrite(address);
    this->clearCE0();
    this->clearRD();
    wait200ns();
    result = this->dataReadHigh();
    this->setRD();
    this->setCE0();
    return result;
}

void Genesis::writeByte(uint16_t address, uint8_t data){
    this->addressWrite(address);
    this->dataSetToOutputs();
    this->dataWriteHigh(data);
    this->clearCE0();
    this->clearWR();
    wait200ns();
    this->setCE0();
    this->setWR();

    // always leave on inputs by default
    this->dataSetToInputs(true);
}

uint16_t Genesis::readWord(uint32_t address){

    uint16_t result;
    this->addressWrite(address);
    this->clearCE0();
    this->clearRD();
    wait200ns();
    result = this->dataReadWordSwapped();
    this->setRD();
    this->setCE0();
    return result;
}
