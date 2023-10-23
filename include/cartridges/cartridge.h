#ifndef CARTRIDGE_H
#define CARTRIDGE_H

class Cartridge{
    public:
        Cartridge();
        virtual ~Cartridge();
        virtual void begin(void);
};

#endif