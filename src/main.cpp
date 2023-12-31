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
#define SD_CLK_DIV  8

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

enum UMDState
{
    TOPLEVEL,
    TOPLEVEL_WAIT_FOR_INPUT,
    CARTRIDGE
};

// const __FlashStringHelper * menuTopLevel[] = {F(" Read Cartridge"), F(" Write Cartridge"), F(" Checksum")};
// const int menuTopLevelSize = 3;

int verifySdCard(int& line);
int verifySdCardSystemSetup(int& line, const char* systemName);

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
        while (1);
    }

    // sd card
    umdDisplay.printf(0, line++, F("init SD card"));
    umdDisplay.redraw();
    SD.setDx(PC8, PC9, PC10, PC11);
    SD.setCMD(PD2);
    SD.setCK(PC12);
    if (!SD.begin(SD_DETECT_PIN))
    {
        umdDisplay.printf(0, line, F("  no card"));
        umdDisplay.redraw();
        while (1);
    }
    // sdCard.setDx(PC8, PC9, PC10, PC11);
    // sdCard.setCMD(PD2);
    // sdCard.setCK(PC12);
    // if(!sdCard.init(SD_DETECT_PIN)){
    //     umdDisplay.printf(0, line, F("  failed"));
    //     umdDisplay.redraw();
    //     while (1);
    // }

    // check that the SD cart is properly formatted for UMDv3
    int sdVerify = verifySdCard(line);
    if(sdVerify != 0)
    {
        umdDisplay.printf(0, line, F("  SD error %d"), sdVerify);
        umdDisplay.redraw();
        while (1);
    }

    // setup onboard mcp23008, GP6 and GP7 LED outputs
    umdDisplay.printf(0, line++, F("init umd MCP23008"));
    umdDisplay.redraw();

    if (!onboardMCP23008.begin(UMD_BOARD_MCP23008_ADDRESS))
    {
        umdDisplay.printf(0, line, F("  failed"));
        umdDisplay.redraw();
        while (1);
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
    umdDisplay.printf(0, line++, F("init cart MCP23008"));
    umdDisplay.redraw();
    if (!adapterMCP23008.begin(UMD_ADAPTER_MCP23008_ADDRESS))
    {
        umdDisplay.printf(0, line, F("  no adapter found"));
        umdDisplay.redraw();
        while (1);
    }

    adapterMCP23008.pinMode(0xFF, INPUT);
    uint8_t adapterId = adapterMCP23008.readGPIO();
    cartridge = cartFactory.getCart(adapterId);
    if (cartridge == nullptr)
    {
        umdDisplay.printf(0, line++, F("  unknown adapter"));
        umdDisplay.redraw();
        while (1);
    }

    //umdDisplay.printf(0, line++, F("  adapter id = %d"), adapterId);
    auto systemName = cartridge->getSystemName();
    umdDisplay.printf(0, line++, F("  %s"), systemName);
    umdDisplay.redraw();

    // check if the sdcard has a _db.txt file containing rom checksums for this system
    sdVerify = verifySdCardSystemSetup(line, systemName);
    if(sdVerify != 0)
    {
        umdDisplay.printf(0, line, F("  SD error %d"), sdVerify);
        umdDisplay.redraw();
        while (1);
    }

    // register callbacks for SerialCommand related to the cartridge
    SCmd.addCommand("scani2c", scmdScanI2C);

    delay(2000);

    // setup the display with 2 layers, having the UMDv3/... static on the first line
    umdDisplay.clear();
    umdDisplay.setLayerLineLength(0, 1);
    umdDisplay.setLayerLineLength(1, UMD_DISPLAY_BUFFER_TOTAL_LINES);
    umdDisplay.printf(0, 0, F("UMDv3/%s"), systemName);
    umdDisplay.setCursorPosition(-1, -1);
    umdDisplay.redraw();

}

void loop()
{
    // Reminder: when debugging ticks isn't accurate at all
    static UMDState umdState = TOPLEVEL;
    static uint32_t currentTicks;
    static Controls userInput;

    // get the ticks
    currentTicks = HAL_GetTick();
    uint8_t inputs = onboardMCP23008.readGPIO();
    userInput.process(inputs, currentTicks);

    switch(umdState)
    {
        case TOPLEVEL_WAIT_FOR_INPUT:
            if(userInput.Down >= userInput.PRESSED)
            {
                umdDisplay.menuCursorUpdate(1, true);
            }
            else if (userInput.Up >= userInput.PRESSED)
            {
                umdDisplay.menuCursorUpdate(-1, true);
            }
            break;
        case TOPLEVEL:
        default:
            auto [items, size] = cartridge->getMenu(0);
            //umdDisplay.initMenu(1, cartridge->getMenuItems(0), cartridge->getMenuSize(0));
            umdDisplay.initMenu(1, items, size);
            umdState = TOPLEVEL_WAIT_FOR_INPUT;
            break;
    }

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

    for (uint8_t address : addresses) { SerialUSB.println(address, HEX); }
}

void inputInterrupt(void)
{
    newInputs = true;
}

int verifySdCard(int& line)
{
    File file = SD.open("/UMD");
    //File file = SD.open("/UMD/Genesis/_db.txt");
    if(!file)
    {
        return 1;
    }

    auto fileName = file.fullname();
    umdDisplay.printf(0, line++, F("  %s"), fileName);
    umdDisplay.redraw();
    file.close();

    return 0;
}

int verifySdCardSystemSetup(int& line, const char* systemName)
{
    String systemStr = systemName;
    String filePath = "/UMD/" + systemStr + "/_db.txt";
    // File file = SD.open(filePath.c_str()); //somehow this doesn't work
    auto path = filePath.c_str();
    File file = SD.open(path, FILE_READ);
    if(!file)
    {
        return 1;
    }

    auto fileName = file.fullname();
    umdDisplay.printf(0, line++, F("  %s"), fileName);
    umdDisplay.redraw();

    int read;
    while(file.available())
    {
        read = file.read();
        SerialUSB.write(read);
    }

    file.close();

    return 0;
}