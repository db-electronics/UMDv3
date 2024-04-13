#pragma once
#include <Arduino.h>

// redefine I2C pins
// https://github.com/stm32duino/Arduino_Core_STM32/wiki/API#default-i2c-pins
const PinMap PinMap_I2C_SDA[] = {
  {PB_9,  I2C1, STM_PIN_DATA(STM_MODE_AF_OD, GPIO_NOPULL, GPIO_AF4_I2C1)},
  {NC,    NP,    0}
};
const PinMap PinMap_I2C_SCL[] = {
  {PB_8,  I2C1, STM_PIN_DATA(STM_MODE_AF_OD, GPIO_NOPULL, GPIO_AF4_I2C1)},
  {NC,    NP,    0}
};
