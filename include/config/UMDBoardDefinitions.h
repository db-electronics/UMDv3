#pragma once

#include <Arduino.h>
#include "services/MCP23008.h"

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
