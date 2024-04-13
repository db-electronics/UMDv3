#include <Arduino.h>
#include <SPI.h>
#include <STM32SD.h>
#include <SerialCommand.h> // https://github.com/db-electronics/ArduinoSerialCommand
#include <SoftwareSerial.h>
#include <USBSerial.h>
#include <Wire.h>
#include <stm32f4xx_hal.h>
#include <stm32f4xx_hal_crc.h>
#include <stm32f4xx_hal_gpio.h>

#include "config/RemapPins.h"
#include "config/UMDBoardDefinitions.h"
#include "services/CartridgeFactory.h"
#include "Controls.h"
#include "services/I2cScanner.h"
#include "services/MCP23008.h"
#include "umdDisplay.h"
#include "services/Crc32Calculator.h"

#ifndef SD_DETECT_PIN
#define SD_DETECT_PIN PD0
#endif

#define SD_CLK_DIV  8

//SdFatFs fatFs;
File sdFile;
SerialCommand SCmd;
MCP23008 onboardMCP23008;
MCP23008 adapterMCP23008;
UMDDisplay umdDisplay;

CartridgeFactory cartFactory;
Crc32Calculator crc32Calculator;
std::unique_ptr<Cartridge> cartridge;
std::vector<const char*> memoryNames;

// SD stuff
int verifySdCard(int& line);
int verifySdCardSystemSetup(int& line, const char* systemName);

void scmdScanI2C(void);

enum UMDUxState
{
    INIT_MAIN_MENU,
    WAIT_FOR_INPUT,
    WAIT_FOR_RELEASE
};

#define UMD_DATA_BUFFER_SIZE_BYTES 512
union DataBuffer{
    uint8_t bytes[UMD_DATA_BUFFER_SIZE_BYTES];
    uint16_t words[UMD_DATA_BUFFER_SIZE_BYTES/2];
    uint32_t dwords[UMD_DATA_BUFFER_SIZE_BYTES/4];
}dataBuffer;


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
    // SD.setDx(PC8, PC9, PC10, PC11);
    // SD.setCMD(PD2);
    // SD.setCK(PC12);
    if (!SD.begin(SD_DETECT_PIN, PC8, PC9, PC10, PC11, PD2, PC12))
    {
        umdDisplay.printf(0, line, F("  no card"));
        umdDisplay.redraw();
        while (1);
    }

    // reduce the SDIO clock, seems more stable like this
    SDIO->CLKCR |= SD_CLK_DIV; 
    
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
    cartridge = cartFactory.GetCart(adapterId, crc32Calculator);
    if (cartridge == nullptr)
    {
        umdDisplay.printf(0, line++, F("  unknown adapter"));
        umdDisplay.redraw();
        while (1);
    }

    //umdDisplay.printf(0, line++, F("  adapter id = %d"), adapterId);
    auto systemName = cartridge->GetSystemName();
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

    // get the supported memory types for this cartridge
    memoryNames = cartridge->GetMemoryNames();

    umdDisplay.setCursorPosition(0, 0);
    umdDisplay.setCursorVisible(true);
    umdDisplay.setClockPosition(20, 7);
    umdDisplay.setClockVisible(false);
    
    umdDisplay.redraw();
}

//MARK: Main loop
void loop()
{
    // Reminder: when debugging ticks isn't accurate at all and SD card is more wonky
    static UMDUxState umdUxState = INIT_MAIN_MENU;
    static Cartridge::CartridgeState umdCartState = Cartridge::IDLE;
    static uint32_t currentTicks=0, previousTicks;
    static uint32_t crc32, cartSize;
    static Controls userInput;
    static UMDMenuIndex currentMenu;
    static int menuIndex, newAmountOfItems;
    UMDActionResult result;

    // get the ticks
    previousTicks = currentTicks;
    currentTicks = HAL_GetTick();

    // process inputs
    uint8_t inputs = onboardMCP23008.readGPIO();
    userInput.process(inputs, currentTicks);

    switch(umdUxState)
    {
        case WAIT_FOR_INPUT:
            if(userInput.Down >= userInput.PRESSED)
            {
                umdDisplay.menuCursorUpdate(1, true);
                umdUxState = WAIT_FOR_RELEASE;
            }
            else if (userInput.Up >= userInput.PRESSED)
            {
                umdDisplay.menuCursorUpdate(-1, true);
                umdUxState = WAIT_FOR_RELEASE;
            }
            else if (userInput.Left >= userInput.PRESSED)
            {
                // scroll line left
                umdUxState = WAIT_FOR_RELEASE;
            }
            else if (userInput.Right >= userInput.PRESSED)
            {
                // scroll line right
                umdUxState = WAIT_FOR_RELEASE;
            }
            else if (userInput.Ok >= userInput.PRESSED){
                // what item is selected?
                int menuItemIndex = umdDisplay.menuCurrentItem();
                
                switch(umdCartState)
                {
                    // At the main menu, offer top level choices
                    case Cartridge::IDLE:
                        switch(menuItemIndex)
                        {
                            // these must match the index of the menu items currently displayed
                            // Identify the connected cartridge by calculating the CRC32 of the ROM and comparing against the database
                            case Cartridge::IDENTIFY:
                                // update state to IDENTIFY,
                                umdCartState = Cartridge::IDENTIFY;
                                cartridge->ResetChecksumCalculator();
                                cartSize = cartridge->GetCartridgeSize();

                                for(int addr = 0; addr < cartSize-1; addr += UMD_DATA_BUFFER_SIZE_BYTES)
                                {
                                    crc32 = cartridge->Identify(addr, dataBuffer.bytes, UMD_DATA_BUFFER_SIZE_BYTES, Cartridge::ReadOptions::HW_CHECKSUM);
                                }
                                umdDisplay.printf(UMD_DISPLAY_LAYER_MENU, 5, F(" SIZE : %08X"), cartSize);
                                umdDisplay.printf(UMD_DISPLAY_LAYER_MENU, 6, F(" CRC32: %08X"), crc32);

                                // TODO search for this crc32 in the database
                                break;
                            case Cartridge::READ: 
                                // update state to READ and offer choice of memory to read from
                                umdCartState = Cartridge::READ;
                                umdDisplay.showMenu(UMD_DISPLAY_LAYER_MENU, memoryNames);
                                break;
                            case Cartridge::WRITE:
                                // update state to WRITE and offer choice of memory to write to
                                umdCartState = Cartridge::WRITE;
                                umdDisplay.showMenu(UMD_DISPLAY_LAYER_MENU, memoryNames);
                                break;
                            default:
                                umdCartState = Cartridge::IDLE;
                                break;
                        }
                        break;
                    case Cartridge::IDENTIFY:
                    case Cartridge::READ:
                    case Cartridge::WRITE:
                        // act on the menu item selected
                        break;
                    default:
                        break;
                }
                // shitty debounce
                umdUxState = WAIT_FOR_RELEASE;
            }
            else if (userInput.Back >= userInput.PRESSED){
                // auto [items, size] = cartridge->getMenu(0);
                // umdDisplay.initMenu(1, items, size);
                umdDisplay.showMenu(UMD_DISPLAY_LAYER_MENU, UMD_MENU_MAIN);
                currentMenu = UMD_MENU_MAIN;
                umdUxState = WAIT_FOR_RELEASE;
            }
            break;
        case WAIT_FOR_RELEASE: // el-cheapo debounce
            if(userInput.Ok == userInput.OFF && userInput.Back == userInput.OFF && userInput.Up == userInput.OFF && userInput.Down == userInput.OFF)
            {
                umdUxState = WAIT_FOR_INPUT;
            }
            break;
        case INIT_MAIN_MENU:
        default:
            // auto [items, size] = cartridge->getMenu(0);
            // umdDisplay.initMenu(1, items, size);
            umdDisplay.showMenu(UMD_DISPLAY_LAYER_MENU, UMD_MENU_MAIN);
            currentMenu = UMD_MENU_MAIN;
            umdCartState = Cartridge::IDLE;
            menuIndex = 0;
            umdUxState = WAIT_FOR_INPUT;
            break;
    }

    umdDisplay.advanceClockAnimation();
    umdDisplay.redraw();
    // SerialUSB.println(F("Tick"));
    // delay(100);
}

void scmdScanI2C(void)
{
    I2cScanner scanner;
    std::vector<uint8_t> addresses;
    addresses = scanner.FindDevices(&Wire);

    for (uint8_t address : addresses) { SerialUSB.println(address, HEX); }
}

//MARK: SD card functions
int verifySdCard(int& line)
{
    sdFile = SD.open("/UMD");
    //File file = SD.open("/UMD/Genesis/_db.txt");
    if(!sdFile)
    {
        return 1;
    }

    auto fileName = sdFile.fullname();
    umdDisplay.printf(0, line++, F("  %s"), fileName);
    umdDisplay.redraw();
    sdFile.close();

    return 0;
}

int verifySdCardSystemSetup(int& line, const char* systemName)
{
    String systemStr = systemName;
    String filePath = "/UMD/" + systemStr + "/_db.txt";
    // File file = SD.open(filePath.c_str()); //somehow this doesn't work
    auto path = filePath.c_str();
    sdFile = SD.open(path);
    if(!sdFile)
    {
        return 1;
    }

    auto fileName = sdFile.fullname();
    umdDisplay.printf(0, line++, F("  %s"), fileName);
    umdDisplay.redraw();

    // test read file contents
    char c[16];
    int i = 0;
    while(sdFile.available())
    {
        c[i++] = sdFile.read();
        SerialUSB.write(c);
    }

    sdFile.close();

    return 0;
}
