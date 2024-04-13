#include "services/CartridgeFactory.h"

std::unique_ptr<Cartridge> CartridgeFactory::GetCart(uint8_t adapterId, IChecksumCalculator& checksumCalculator){
    switch(adapterId){
        case GENESIS:
            return std::make_unique<Genesis>(checksumCalculator);
        default:
            return nullptr;

    }
    return nullptr;
}