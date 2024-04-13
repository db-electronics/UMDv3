#pragma once

#include "cartridge.h"

class Undefined : public Cartridge
{
    public:
        Undefined();
        virtual void begin(void);
};
