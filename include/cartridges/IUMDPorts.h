#ifndef IUMDPORTS_H
#define IUMDPORTS_H

#include <Arduino.h>

class IUMDPorts{

    public:
        virtual ~IUMDPorts() {};
        virtual void setDefaults();

        void waitNs(uint16_t nanoSeconds);

        void addressWrite(uint32_t address);
        void addressWrite(uint16_t address);

        uint8_t dataReadLow();
        uint8_t dataReadHigh();
        uint16_t dataReadWord();
        uint16_t dataReadWordSwapped();

        void dataWrite(uint8_t value);
        void dataWriteLow(uint8_t value);
        void dataWriteHigh(uint8_t value);
        void dataWrite(uint16_t value);
        void dataWriteSwapped(uint16_t value);
        void dataSetToInputs(bool pullups);
        void dataSetToOutputs();

        void setCE0();
        void setCE1();
        void setCE2();
        void setCE3();
        void setWR();
        void setRD();

        void clearCE0();
        void clearCE1();
        void clearCE2();
        void clearCE3();
        void clearRD();
        void clearWR();
};

#endif