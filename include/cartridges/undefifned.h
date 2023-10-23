#ifndef UNDEFINED_H
#define UNDEFINED_h

#include "cartridge.h"

class Undefined : public Cartridge
{
    public:
        Undefined();
        virtual void begin(void);
};

#endif