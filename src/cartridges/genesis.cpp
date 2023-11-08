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
    addressWrite(address);
    clearCE0();
    clearRD();
    wait200ns();
    result = dataReadHigh();
    setRD();
    setCE0();
    return result;
}

void Genesis::writeByte(uint16_t address, uint8_t data){
    addressWrite(address);
    dataSetToOutputs();
    dataWriteHigh(data);
    clearCE0();
    clearWR();
    wait200ns();
    setCE0();
    setWR();

    // always leave on inputs by default
    dataSetToInputs(true);
}

uint16_t Genesis::readWord(uint32_t address){

    uint16_t result;
    addressWrite(address);
    clearCE0();
    clearRD();
    wait200ns();
    result = dataReadWordSwapped();
    setRD();
    setCE0();
    return result;
}
