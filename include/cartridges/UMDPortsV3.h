#ifndef UMDPORTSV3_H
#define UMDPORTSV3_H

#include "IUMDPorts.h"
#include <stm32f4xx_hal.h>
#include <stm32f4xx_hal_gpio.h>

#define UMD_PORT_ADDR_LOW       GPIOA
#define UMD_PORT_ADDR_MID       GPIOC
#define UMD_PORT_ADDR_HIGH      GPIOD

#define UMD_PORT_DATABUS        GPIOE

#define UMD_PORT_CE0            GPIOC
#define UMD_PIN_CE0             GPIO_PIN_13

#define UMD_PORT_CE1            GPIOD
#define UMD_PIN_CE1             GPIO_PIN_5
#define UMD_PORT_CE2            GPIOD
#define UMD_PIN_CE2             GPIO_PIN_6
#define UMD_PORT_CE3            GPIOD
#define UMD_PIN_CE3             GPIO_PIN_7

#define UMD_PORT_RD             GPIOC
#define UMD_PIN_RD              GPIO_PIN_14
#define UMD_PORT_WR             GPIOC
#define UMD_PIN_WR              GPIO_PIN_15

#define UMD_PORT_IO0            GPIOB
#define UMD_PIN_IO0             GPIO_PIN_0
#define UMD_PORT_IO1            GPIOB
#define UMD_PIN_IO1             GPIO_PIN_1
#define UMD_PORT_IO2            GPIOB
#define UMD_PIN_IO2             GPIO_PIN_5
#define UMD_PORT_IO3            GPIOB
#define UMD_PIN_IO3             GPIO_PIN_6
#define UMD_PORT_IO4            GPIOB
#define UMD_PIN_IO4             GPIO_PIN_13
#define UMD_PORT_IO5            GPIOB
#define UMD_PIN_IO5             GPIO_PIN_12
#define UMD_PORT_IO6            GPIOB
#define UMD_PIN_IO6             GPIO_PIN_15
#define UMD_PORT_IO7            GPIOB
#define UMD_PIN_IO7             GPIO_PIN_14
#define UMD_PORT_IO8            GPIOB
#define UMD_PIN_IO8             GPIO_PIN_7


#define UMD_PORT_SWAP_BYTES(w)          (w << 8) || (w >> 8)

class UMDPortsV3 : public IUMDPorts{
    
    public:
        void setDefaults();

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
        void setIO(uint8_t io);

        void clearCE0();
        void clearCE1();
        void clearCE2();
        void clearCE3();
        void clearRD();
        void clearWR();
        void clearIO(uint8_t io);
  
        void ioSetToOutput(uint16_t io, bool pushpull);
        void ioSetToInput(uint16_t io, bool pullup);

        uint8_t ioRead(uint8_t io);

    private:
        uint16_t ticks;
        void _bitSet(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin);
        void _bitClear(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin);

        void _bitSetToOutput(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin, bool pushpull);
        void _bitSetToInput(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin, bool pullup);

        uint8_t _bitRead(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin);

        void _portSetToInput(GPIO_TypeDef *GPIOx, bool pullups);
        void _portSetToOutput(GPIO_TypeDef *GPIOx);

        void _portByteWriteLow(GPIO_TypeDef *GPIOx, uint8_t value);
        uint8_t _portByteReadLow(GPIO_TypeDef *GPIOx);
        void _portByteWriteHigh(GPIO_TypeDef *GPIOx, uint8_t value);
        uint8_t _portByteReadHigh(GPIO_TypeDef *GPIOx);

        void _portWordWrite(GPIO_TypeDef *GPIOx, uint16_t value);
        uint16_t _portWordRead(GPIO_TypeDef *GPIOx);

};

#endif