#pragma once

#include <Arduino.h>
#include "services/MCP23008.h"

using namespace i2cdevice;

// #define UMD_BOARD_PUSHBUTTONS           Mcp23008::Pins::GP0 | Mcp23008::Pins::GP1 | Mcp23008::Pins::GP2 | Mcp23008::Pins::GP3 | Mcp23008::Pins::GP4 | Mcp23008::Pins::GP5
// #define UMD_BOARD_LEDS                  Mcp23008::Pins::GP6 | Mcp23008::Pins::GP7
// #define UMD_MCP23008_INTERRUPT_PIN      PD1
// #define UMD_LEFT_PUSHBUTTON             Mcp23008::Pins::GP0
// #define UMD_DOWN_PUSHBUTTON             Mcp23008::Pins::GP1
// #define UMD_UP_PUSHBUTTON               Mcp23008::Pins::GP2
// #define UMD_RIGHT_PUSHBUTTON            Mcp23008::Pins::GP3
// #define UMD_OK_PUSHBUTTON               Mcp23008::Pins::GP4
// #define UMD_BACK_PUSHBUTTON             Mcp23008::Pins::GP5
// #define UMD_LED_0                       Mcp23008::Pins::GP6
// #define UMD_LED_1                       Mcp23008::Pins::GP7
