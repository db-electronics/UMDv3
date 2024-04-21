#include "services/CartridgeFactory.h"

std::unique_ptr<Cartridge> CartridgeFactory::GetCart(uint8_t adapterId){
    switch(adapterId){
        case GENESIS:
            return std::make_unique<Genesis>(crc32Calculator);
        default:
            return nullptr;

    }
    return nullptr;
}