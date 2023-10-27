#ifndef CARTRIDGE_H
#define CARTRIDGE_H

#include "umdPorts.h"

class Cartridge : public umdPorts{
    public:
        Cartridge();
        virtual ~Cartridge();
        virtual void begin(void);
};

#endif