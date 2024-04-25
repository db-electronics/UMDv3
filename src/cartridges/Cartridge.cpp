#include "cartridges/Cartridge.h"

cartridges::Cartridge::Cartridge(IChecksumCalculator& checksumCalculator)
    : mChecksumCalculator(checksumCalculator) {
    setDefaults();
}

cartridges::Cartridge::~Cartridge(){
}

void cartridges::Cartridge::TestWait(void){
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

void cartridges::Cartridge::ResetChecksumCalculator(){
    mChecksumCalculator.Reset();
}
