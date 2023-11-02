#include <Arduino.h>
#include <SPI.h>
#include <STM32SD.h>
#include <SerialCommand.h> // https://github.com/db-electronics/ArduinoSerialCommand
#include <SoftwareSerial.h>
#include <USBSerial.h>
#include <Wire.h>
#include <stm32f4xx_hal.h>
#include <stm32f4xx_hal_gpio.h>

#include "CartridgeFactory.h"
#include "Controls.h"
#include "i2cScanner.h"
#include "mcp23008.h"
#include "pinRemap.h"
#include "umdBoardDefinitions.h"
#include "umdDisplay.h"

#ifndef SD_DETECT_PIN
#define SD_DETECT_PIN PD0
#endif

SdFatFs fatFs;
SerialCommand SCmd;
MCP23008 onboardMCP23008;
MCP23008 adapterMCP23008;
UMDDisplay umdDisplay;

CartridgeFactory cartFactory;
std::unique_ptr<Cartridge> cartridge;

void scmdScanI2C(void);
void inputInterrupt(void);

bool newInputs;

int displayYOffset = 1;
int cursorX = 0, cursorY = 1;

void setup()
{
    int line = 0;
    // setup USB serial
    // https://primalcortex.wordpress.com/2020/10/11/stm32-blue-pill-board-arduino-core-and-usb-serial-output-on-platformio/
    SerialUSB.begin(460800); // no need for a parameter here maybe?
    pinMode(PA10, OUTPUT);
    digitalWrite(PA10, LOW);
    delay(10);
    digitalWrite(PA10, HIGH);
    SerialUSB.println(F("UMDv3"));

    // setup I2C
    // https://github.com/stm32duino/Arduino_Core_STM32/wiki/API#i2c
    // Wire.setSCL(PB8);
    // Wire.setSDA(PB9);
    Wire.begin();

    if (!umdDisplay.begin())
    {
        SerialUSB.println(F("display init failure"));
        while (1)
            ;
    }

    // sd card
    umdDisplay.printf(line++, F("init SD card"));
    umdDisplay.redraw();
    SD.setDx(PC8, PC9, PC10, PC11);
    SD.setCMD(PD2);
    SD.setCK(PC12);
    if (!SD.begin(SD_DETECT_PIN))
    {
        umdDisplay.printf(line, F("  failed"));
        umdDisplay.redraw();
        while (1)
            ;
    }

    // setup onboard mcp23008, GP6 and GP7 LED outputs
    umdDisplay.printf(line++, F("init umd MCP23008"));
    umdDisplay.redraw();

    if (!onboardMCP23008.begin(UMD_BOARD_MCP23008_ADDRESS))
    {
        umdDisplay.printf(line, F("  failed"));
        umdDisplay.redraw();
        while (1)
            ;
    }

    onboardMCP23008.pinMode(UMD_BOARD_LEDS, OUTPUT);
    onboardMCP23008.setPullUpResistors(UMD_BOARD_PUSHBUTTONS, true);
    onboardMCP23008.setInterruptOnChange(UMD_BOARD_PUSHBUTTONS, true);
    onboardMCP23008.setInterruptControl(UMD_BOARD_PUSHBUTTONS, onboardMCP23008.PREVIOUS);
    onboardMCP23008.setInterruptEnable(UMD_BOARD_PUSHBUTTONS, true);
    onboardMCP23008.digitalWrite(UMD_BOARD_LEDS, LOW);
    newInputs = false;

    // interrupt line tied to PD1
    // pinMode(UMD_MCP23008_INTERRUPT_PIN, INPUT);
    // attachInterrupt(digitalPinToInterrupt(UMD_MCP23008_INTERRUPT_PIN),
    // inputInterrupt, FALLING);

    // setup adapter mcp23008, read adapter id
    umdDisplay.printf(line++, F("init cart MCP23008"));
    umdDisplay.redraw();
    if (!adapterMCP23008.begin(UMD_ADAPTER_MCP23008_ADDRESS))
    {
        umdDisplay.printf(line, F(" -no adapter found"));
        umdDisplay.redraw();
        while (1)
            ;
    }

    adapterMCP23008.pinMode(0xFF, INPUT);
    uint8_t adapterId = adapterMCP23008.readGPIO();
    cartridge = cartFactory.getCart(adapterId);
    if (cartridge == nullptr)
    {
        umdDisplay.printf(line++, F(" -unknown adapter"));
        umdDisplay.redraw();
        while (1);
    }

    umdDisplay.printf(line++, F(" -adapter id = %d"), adapterId);
    umdDisplay.printf(line++, F(" -%s"), cartridge->getSystemName());
    umdDisplay.redraw();

    // register callbacks for SerialCommand related to the cartridge
    SCmd.addCommand("scani2c", scmdScanI2C);

    delay(2000);

    umdDisplay.clear();
    umdDisplay.printf(0, F("UMDv3/%s"),
                      cartridge->getSystemName());
    umdDisplay.setCursorPosition(-1, -1);
    umdDisplay.redraw();
}

void loop()
{
    // Reminder: when debugging ticks isn't accurate at all
    static uint32_t currentTicks = HAL_GetTick();
    static Controls userInput;

    uint8_t inputs = onboardMCP23008.readGPIO();
    userInput.process(inputs, currentTicks);
    
    cartridge->testWait();
    //cartridge->testWaitNs();

    umdDisplay.redraw();
    delay(100);
}

void scmdScanI2C(void)
{
    // char *arg;
    // uint32_t address, value;

    // this is the address to poke
    // arg = SCmd.next();
    // address = (uint32_t)strtoul(arg, (char**)0, 0);

    // value = *(__IO uint32_t *)(address);
    // SerialUSB.print(value, HEX);
    I2CScanner scanner;
    std::vector<uint8_t> addresses;
    addresses = scanner.findDevices(&Wire);

    for (uint8_t address : addresses)
    {
        SerialUSB.println(address, HEX);
    }
}

void inputInterrupt(void)
{
    newInputs = true;
}