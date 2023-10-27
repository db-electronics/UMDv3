#include "umdPorts.h"



void umdPorts::begin(){
    this->setDefaults();
}

void umdPorts::setDefaults(){

    GPIO_InitTypeDef GPIO_InitStruct = {0};

    // enable master clock output on MCo1 PA8
    GPIO_InitStruct.Pin = GPIO_PIN_8;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF0_MCO;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    HAL_RCC_MCOConfig(RCC_MCO1, RCC_MCO1SOURCE_HSE, RCC_MCODIV_4);

    //A0 to A7 on PA0 to PA
    GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    //A8 to A15 on PC0 to PC7
    GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

    //A16 to A23 on PD8 to PD15
    GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

    //D0 to D15 on PE0 to PE15
    GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7 | GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

    //CE0 on PC13
    GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = UMD_PIN_CE0;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(UMD_PORT_CE0, &GPIO_InitStruct);
    this->bitSet(UMD_PORT_CE0, UMD_PIN_CE0);
    //GPIOB->BSRR |= (1<<7);

    //CE1 - CE3 on PD5, PD6, PD7
    GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);
    this->bitSet(UMD_PORT_CE1, UMD_PIN_CE0);
    this->bitSet(UMD_PORT_CE2, UMD_PIN_CE0);
    this->bitSet(UMD_PORT_CE3, UMD_PIN_CE0);

    // RD and WR on PC14, PC15
    GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = GPIO_PIN_14 | GPIO_PIN_15;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);


  // C:\Users\rrichard\.platformio\packages\framework-arduinoststm32\variants\STM32F4xx\F407V(E-G)T_F417V(E-G)T\PeripheralPins.c
  // pinMode(PB0, OUTPUT);
  // pinMode(PB7, OUTPUT);

  // set PB7 as output
  // https://controllerstech.com/stm32-gpio-output-config-using-registers/
  // GPIOB->MODER |= (1<<14); // output mode
  // GPIOB->OTYPER &= ~(1<<7); // push pull mode
  // GPIOB->OSPEEDR |= (1<<15); // fast speed
  // GPIOB->PUPDR &= ~(3<<7); // no pull-up or pull-down


  GPIO_InitStruct.Pin = GPIO_PIN_7;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

   // set the bit
  GPIOB->BSRR |= (1<<(7+16)); // reset the bit

}

void umdPorts::bitSet(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin){
    GPIOx->BSRR = GPIO_Pin;
}

void umdPorts::bitClear(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin){
    GPIOx->BSRR = (uint32_t)GPIO_Pin << 16U;
}