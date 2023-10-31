#include "cartridges/genesis.h"

Genesis::Genesis(){}

Genesis::~Genesis(){}

const char* Genesis::getSystemName(){
    return "Genesis";
}

uint8_t Genesis::readByte(uint32_t address){

    uint8_t result;
    this->addressWrite(address);
    this->waitNs(addressSetupTime);
    this->clearCE0();
    this->clearRD();
    this->waitNs(readHoldTime);
    result = this->dataReadHigh();
    this->setRD();
    this->setCE0();
    return result;
}

void Genesis::writeByte(uint16_t address, uint8_t data){
    this->addressWrite(address);
    this->waitNs(addressSetupTime);
    this->dataSetToOutputs();
    this->dataWriteHigh(data);
    this->clearCE0();
    this->clearWR();
    this->waitNs(writeHoldTime);
    this->setCE0();
    this->setWR();

    // always leave on inputs by default
    this->dataSetToInputs(true);
}

uint16_t Genesis::readWord(uint32_t address){

    uint16_t result;
    this->addressWrite(address);
    this->waitNs(addressSetupTime);
    this->clearCE0();
    this->clearRD();
    this->waitNs(readHoldTime);
    result = this->dataReadWordSwapped();
    this->setRD();
    this->setCE0();
    return result;
}
