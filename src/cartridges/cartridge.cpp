#include "cartridges/cartridge.h"

Cartridge::Cartridge(IUMDPorts *umdPorts){
    _umdPorts = umdPorts;
    _umdPorts->setDefaults();
}

Cartridge::~Cartridge(){
}

uint8_t Cartridge::readByte(uint16_t address){

    uint8_t result;
    _umdPorts->addressWrite(address);
    _umdPorts->waitNs(addressSetupTime);
    _umdPorts->dataSetToInputs(true);
    _umdPorts->clearCE0();
    _umdPorts->clearRD();
    _umdPorts->waitNs(readHoldTime);
    result = _umdPorts->dataReadLow();
    _umdPorts->setRD();
    _umdPorts->setCE0();
    return  
    result;
}

uint8_t Cartridge::readByte(uint32_t address){

    uint8_t result;
    _umdPorts->addressWrite(address);
    _umdPorts->waitNs(addressSetupTime);
    _umdPorts->dataSetToInputs(true);
    _umdPorts->clearCE0();
    _umdPorts->clearRD();
    _umdPorts->waitNs(readHoldTime);
    result = _umdPorts->dataReadLow();
    _umdPorts->setRD();
    _umdPorts->setCE0();
    return result;
}