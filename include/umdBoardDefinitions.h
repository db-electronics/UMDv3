#ifndef UMDBOARDDEFINITIONS_H
#define UMDBOARDDEFINITIONS_H

#include <Arduino.h>
#include "mcp23008.h"

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

#define UMD_BOARD_MCP23008_ADDRESS      0x27
#define UMD_ADAPTER_MCP23008_ADDRESS    0x20
#define UMD_BOARD_PUSHBUTTONS           MCP23008_GP0 | MCP23008_GP1 | MCP23008_GP2 | MCP23008_GP3 | MCP23008_GP4 | MCP23008_GP5
#define UMD_BOARD_LEDS                  MCP23008_GP6 | MCP23008_GP7
#define UMD_MCP23008_INTERRUPT_PIN      PD1
#define UMD_LEFT_PUSHBUTTON             MCP23008_GP0
#define UMD_DOWN_PUSHBUTTON             MCP23008_GP1
#define UMD_UP_PUSHBUTTON               MCP23008_GP2
#define UMD_RIGHT_PUSHBUTTON            MCP23008_GP3
#define UMD_OK_PUSHBUTTON               MCP23008_GP4
#define UMD_BACK_PUSHBUTTON             MCP23008_GP5
#define UMD_LED_0                       MCP23008_GP6
#define UMD_LED_1                       MCP23008_GP7



#endif