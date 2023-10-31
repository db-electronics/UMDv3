#ifndef GENESIS_H
#define GENESIS_H

#include "IUMDPorts.h"
#include "cartridge.h"
#include <string>

class Genesis : public Cartridge {
    public:
        const String SystemName = "Genesis";
        Genesis();
        virtual ~Genesis();

};

#endif