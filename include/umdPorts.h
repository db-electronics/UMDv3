#ifndef UMDPORTS_H
#define UMDPORTS_H

#include <stm32f4xx_hal.h>
#include <stm32f4xx_hal_gpio.h>

#define UMD_PORT_ADDR_LOW GPIOA
#define UMD_PORT_ADDR_MID GPIOC
#define UMD_PORT_ADDR_HIGH GPIOD

#define UMD_PORT_DATA GPIOE

#define UMD_PORT_CE0 GPIOC
#define UMD_PIN_CE0  GPIO_PIN_13

#define UMD_PORT_CE1 GPIOD
#define UMD_PIN_CE1  GPIO_PIN_5
#define UMD_PORT_CE2 GPIOD
#define UMD_PIN_CE2  GPIO_PIN_6
#define UMD_PORT_CE3 GPIOD
#define UMD_PIN_CE3  GPIO_PIN_7

#define UMD_PORT_RD GPIOC
#define UMD_PIN_RD  GPIO_PIN_14
#define UMD_PORT_WR GPIOC
#define UMD_PIN_WR  GPIO_PIN_15

#define UMD_PORT_SWAP_BYTES(w)          (w << 8) || (w >> 8)

#define UMD_PORT_TIMER_COUNT            (TIM7->CNT)
#define UMD_PORT_TIMER_PERIOD_NS        (1000/168)
#define UMD_PORT_TIMER_NS_TO_TICKS(ns)  (ns/UMD_PORT_TIMER_PERIOD_NS)
#define UMD_PORT_WAIT_NS(ns) {\
            TIM7->CNT = 0;\
            ticks = ns/(1000/168);\
            while(TIM7->CNT <= ticks);\
        }

class umdPorts{

    public:
        umdPorts();
        void setDefaults();

    // the interface which inhereting classes can talk to the ports
    // no MCU specific code here!
    protected:
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

    // MCU specific code
    private:
        void _portBitSet(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin);
        void _portBitClear(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin);

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