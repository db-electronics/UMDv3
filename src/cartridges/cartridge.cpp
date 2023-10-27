#include "cartridges/cartridge.h"

Cartridge::Cartridge(){
    this->setDefaults();
}

Cartridge::~Cartridge(){
}

uint8_t Cartridge::readByte(uint16_t address){
    // TODO
    // add timing, slow it down for 200ns ROM
    uint8_t result;
    this->addressWrite(address);
    this->dataSetToInputs(true);
    this->clearCE0();
    this->clearRD();
    result = this->dataReadLow();
    this->setRD();
    this->setCE0();
    return result;
}