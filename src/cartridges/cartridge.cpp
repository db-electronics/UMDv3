#include "cartridges/Cartridge.h"

Cartridge::Cartridge(IChecksumCalculator& checksumCalculator)
    : mChecksumCalculator(checksumCalculator) {
    setDefaults();
}

Cartridge::~Cartridge(){
}

void Cartridge::TestWait(void){
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

void Cartridge::ResetChecksumCalculator(){
    mChecksumCalculator.Reset();
}
