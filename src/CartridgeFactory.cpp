#include "CartridgeFactory.h"

Cartridge* CartridgeFactory::getCart(uint8_t adapterId){
    UMDAdapterType system = static_cast<UMDAdapterType>(adapterId);
    switch(system){
        case GENESIS:
            return new Genesis();
            break;
        default:
            return nullptr;
            break;
    }
    return nullptr;
}