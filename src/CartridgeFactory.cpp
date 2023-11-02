#include "CartridgeFactory.h"

std::unique_ptr<Cartridge> CartridgeFactory::getCart(uint8_t adapterId){
    //UMDAdapterType system = static_cast<UMDAdapterType>(adapterId);
    switch(adapterId){
        case GENESIS:
            return std::make_unique<Genesis>();
        default:
            return nullptr;

    }
    return nullptr;
}