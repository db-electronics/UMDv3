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
#define UMD_PORT_WR GPIOC

#define UMD_PORT_BIT_SET(p, b) (p->BSSR |= (1<<b))

class umdPorts{

    public:
        void begin();
        void setDefaults();

        void bitSet(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin);
        void bitClear(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin);

        void byteWrite(GPIO_TypeDef *GPIOx, uint8_t value);
};

#endif