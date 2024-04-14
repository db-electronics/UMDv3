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

#include "Umd.h"
#include "config/RemapPins.h"
#include "config/UMDBoardDefinitions.h"
#include "services/I2cScanner.h"
#include "services/Crc32Calculator.h"

#ifndef SD_DETECT_PIN
#define SD_DETECT_PIN PD0
#endif

#define SD_CLK_DIV  8

//SdFatFs fatFs;
File sdFile;
SerialCommand SCmd;

// TODO move all this to UMD.h
Crc32Calculator crc32Calculator;

// SD stuff
int verifySdCard(int& line);
int verifySdCardSystemSetup(int& line, const char* systemName);

void scmdScanI2C(void);

// MARK: Setup
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

    if (!Umd::Display.begin())
    {
        SerialUSB.println(F("display init failure"));
        while (1);
    }

    // sd card
    Umd::Display.printf(0, line++, F("init SD card"));
    Umd::Display.redraw();
    // SD.setDx(PC8, PC9, PC10, PC11);
    // SD.setCMD(PD2);
    // SD.setCK(PC12);
    if (!SD.begin(SD_DETECT_PIN, PC8, PC9, PC10, PC11, PD2, PC12))
    {
        Umd::Display.printf(0, line, F("  no card"));
        Umd::Display.redraw();
        while (1);
    }

    // reduce the SDIO clock, seems more stable like this
    SDIO->CLKCR |= SD_CLK_DIV; 
    
    // check that the SD cart is properly formatted for UMDv3
    int sdVerify = verifySdCard(line);
    if(sdVerify != 0)
    {
        Umd::Display.printf(0, line, F("  SD error %d"), sdVerify);
        Umd::Display.redraw();
        while (1);
    }

    // setup onboard mcp23008, GP6 and GP7 LED outputs
    Umd::Display.printf(0, line++, F("init umd MCP23008"));
    Umd::Display.redraw();

    if (!Umd::IoExpander.begin(UMD_BOARD_MCP23008_ADDRESS))
    {
        Umd::Display.printf(0, line, F("  failed"));
        Umd::Display.redraw();
        while (1);
    }

    Umd::IoExpander.pinMode(UMD_BOARD_LEDS, OUTPUT);
    Umd::IoExpander.setPullUpResistors(UMD_BOARD_PUSHBUTTONS, true);
    Umd::IoExpander.setInterruptOnChange(UMD_BOARD_PUSHBUTTONS, true);
    Umd::IoExpander.setInterruptControl(UMD_BOARD_PUSHBUTTONS, Umd::IoExpander.PREVIOUS);
    Umd::IoExpander.setInterruptEnable(UMD_BOARD_PUSHBUTTONS, true);
    Umd::IoExpander.digitalWrite(UMD_BOARD_LEDS, LOW);

    // setup adapter mcp23008, read adapter id
    Umd::Display.printf(0, line++, F("init cart MCP23008"));
    Umd::Display.redraw();
    if (!Umd::Cart::IoExpander.begin(UMD_ADAPTER_MCP23008_ADDRESS))
    {
        Umd::Display.printf(0, line, F("  no adapter found"));
        Umd::Display.redraw();
        while (1);
    }

    Umd::Cart::IoExpander.pinMode(0xFF, INPUT);
    uint8_t adapterId = Umd::Cart::IoExpander.readGPIO();

    Umd::Cart::pCartridge = Umd::Cart::Factory.GetCart(adapterId, crc32Calculator);
    if (Umd::Cart::pCartridge == nullptr)
    {
        Umd::Display.printf(0, line++, F("  unknown adapter"));
        Umd::Display.redraw();
        while (1);
    }

    //Umd::Display.printf(0, line++, F("  adapter id = %d"), adapterId);
    auto systemName = Umd::Cart::pCartridge->GetSystemName();
    Umd::Display.printf(0, line++, F("  %s"), systemName);
    Umd::Display.redraw();

    // check if the sdcard has a _db.txt file containing rom checksums for this system
    sdVerify = verifySdCardSystemSetup(line, systemName);
    if(sdVerify != 0)
    {
        Umd::Display.printf(0, line, F("  SD error %d"), sdVerify);
        Umd::Display.redraw();
        while (1);
    }

    // register callbacks for SerialCommand related to the cartridge
    SCmd.addCommand("scani2c", scmdScanI2C);

    delay(2000);

    // setup the display with 2 layers, having the UMDv3/... static on the first line
    Umd::Display.clear();
    Umd::Display.setLayerLineLength(0, 1);
    Umd::Display.setLayerLineLength(1, UMD_DISPLAY_BUFFER_TOTAL_LINES);
    Umd::Display.printf(0, 0, F("UMDv3/%s"), systemName);

    // get the supported memory types for this cartridge
    Umd::Cart::MemoryNames = Umd::Cart::pCartridge->GetMemoryNames();

    Umd::Display.setCursorPosition(0, 0);
    Umd::Display.setCursorVisible(true);
    Umd::Display.setClockPosition(20, 7);
    Umd::Display.setClockVisible(false);
    
    Umd::Display.redraw();

    Umd::Ux::State = Umd::Ux::INIT_MAIN_MENU;
    Umd::Cart::State = Cartridge::IDLE;
}

//MARK: Main loop
void loop()
{
    // Reminder: when debugging ticks isn't accurate at all and SD card is more wonky
    static uint32_t currentTicks=0, previousTicks;
    static uint32_t cartSize;

    static UMDMenuIndex currentMenu;

    // get the ticks
    previousTicks = currentTicks;
    currentTicks = HAL_GetTick();

    // process inputs
    uint8_t inputs = Umd::IoExpander.readGPIO();
    Umd::Ux::UserInput.process(inputs, currentTicks);

    switch(Umd::Ux::State)
    {
        case Umd::Ux::WAIT_FOR_INPUT:
            if(Umd::Ux::UserInput.Down >= Umd::Ux::UserInput.PRESSED)
            {
                Umd::Display.menuCursorUpdate(1, true);
                Umd::Ux::State = Umd::Ux::WAIT_FOR_RELEASE;
            }
            else if (Umd::Ux::UserInput.Up >= Umd::Ux::UserInput.PRESSED)
            {
                Umd::Display.menuCursorUpdate(-1, true);
                Umd::Ux::State = Umd::Ux::WAIT_FOR_RELEASE;
            }
            else if (Umd::Ux::UserInput.Left >= Umd::Ux::UserInput.PRESSED)
            {
                // TODO scroll line left
                Umd::Ux::State = Umd::Ux::WAIT_FOR_RELEASE;
            }
            else if (Umd::Ux::UserInput.Right >= Umd::Ux::UserInput.PRESSED)
            {
                // TODO scroll line right
                Umd::Ux::State = Umd::Ux::WAIT_FOR_RELEASE;
            }
            else if (Umd::Ux::UserInput.Ok >= Umd::Ux::UserInput.PRESSED){
                // what item is selected?
                int menuItemIndex = Umd::Display.menuCurrentItem();
                
                switch(Umd::Cart::State)
                {
                    // At the main menu, offer top level choices
                    case Cartridge::IDLE:
                        switch(menuItemIndex)
                        {
                            // these must match the index of the menu items currently displayed
                            // Identify the connected cartridge by calculating the CRC32 of the ROM and comparing against the database
                            case Cartridge::IDENTIFY:
                                // update state to IDENTIFY,
                                Umd::Cart::State = Cartridge::IDENTIFY;
                                currentTicks = HAL_GetTick();
                                Umd::Cart::pCartridge->ResetChecksumCalculator();
                                cartSize = Umd::Cart::pCartridge->GetCartridgeSize();

                                for(int addr = 0; addr < cartSize-1; addr += Umd::BUFFER_SIZE_BYTES)
                                {
                                    Umd::Cart::pCartridge->Identify(addr, Umd::DataBuffer.bytes, Umd::BUFFER_SIZE_BYTES, Cartridge::ReadOptions::HW_CHECKSUM);
                                }

                                Umd::OperationTime = HAL_GetTick() - currentTicks;

                                // ask cartridge for some metadata about the ROM
                                Umd::Cart::Metadata.clear();
                                Umd::Cart::Metadata = Umd::Cart::pCartridge->GetMetadata();

                                Umd::UpdateDisplayPathAddressBar("Id");
                                Umd::Display.showMenu(UMD_DISPLAY_LAYER_MENU, Umd::Cart::Metadata);
                                Umd::Display.AddMenuItem(F("Size : %08X"), cartSize);
                                Umd::Display.AddMenuItem(F("CRC32: %08X"), Umd::Cart::pCartridge->GetAccumulatedChecksum());

                                // TODO search for this crc32 in the database
                                Umd::Cart::State = Cartridge::IDLE;
                                Umd::Ux::State = Umd::Ux::WAIT_FOR_INPUT;
                                break;
                            case Cartridge::READ: 
                                // update state to READ and offer choice of memory to read from
                                Umd::Cart::State = Cartridge::READ;
                                Umd::UpdateDisplayPathAddressBar("Read");
                                Umd::Display.showMenu(UMD_DISPLAY_LAYER_MENU, Umd::Cart::MemoryNames);
                                Umd::Ux::State = Umd::Ux::WAIT_FOR_RELEASE;
                                break;
                            case Cartridge::WRITE:
                                // update state to WRITE and offer choice of memory to write to
                                Umd::Cart::State = Cartridge::WRITE;
                                Umd::UpdateDisplayPathAddressBar("Write");
                                Umd::Display.showMenu(UMD_DISPLAY_LAYER_MENU, Umd::Cart::MemoryNames);
                                Umd::Ux::State = Umd::Ux::WAIT_FOR_RELEASE;
                                break;
                            default:
                                Umd::Cart::State = Cartridge::IDLE;
                                Umd::Ux::State = Umd::Ux::WAIT_FOR_RELEASE;
                                break;
                        }
                        break;
                    case Cartridge::READ:
                        // keep on reading
                        // finished the read
                        Umd::Cart::State = Cartridge::IDLE;
                        Umd::Ux::State = Umd::Ux::WAIT_FOR_INPUT;
                        break;
                    case Cartridge::WRITE:
                        // keep on writing
                        // finished the write
                        Umd::Cart::State = Cartridge::IDLE;
                        Umd::Ux::State = Umd::Ux::WAIT_FOR_INPUT;
                        break;
                    default:
                        Umd::Cart::State = Cartridge::IDLE;
                        Umd::Ux::State = Umd::Ux::WAIT_FOR_INPUT;
                        break;
                }
            }
            else if (Umd::Ux::UserInput.Back >= Umd::Ux::UserInput.PRESSED){
                Umd::UpdateDisplayPathAddressBar("");
                Umd::Display.showMenu(UMD_DISPLAY_LAYER_MENU, UMD_MENU_MAIN);
                currentMenu = UMD_MENU_MAIN;
                Umd::Ux::State = Umd::Ux::WAIT_FOR_RELEASE;
            }
            break;
        case Umd::Ux::WAIT_FOR_RELEASE: // el-cheapo debounce
            if(Umd::Ux::UserInput.Ok == Umd::Ux::UserInput.OFF && Umd::Ux::UserInput.Back == Umd::Ux::UserInput.OFF && Umd::Ux::UserInput.Up == Umd::Ux::UserInput.OFF && Umd::Ux::UserInput.Down == Umd::Ux::UserInput.OFF)
            {
                Umd::Ux::State = Umd::Ux::WAIT_FOR_INPUT;
            }
            break;
        case Umd::Ux::INIT_MAIN_MENU:
        default:
            Umd::UpdateDisplayPathAddressBar("Id");
            Umd::Display.showMenu(UMD_DISPLAY_LAYER_MENU, UMD_MENU_MAIN);
            currentMenu = UMD_MENU_MAIN;
            Umd::Cart::State = Cartridge::IDLE;
            Umd::Ux::State = Umd::Ux::WAIT_FOR_INPUT;
            break;
    }

    Umd::Display.advanceClockAnimation();
    Umd::Display.redraw();
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
    Umd::Display.printf(0, line++, F("  %s"), fileName);
    Umd::Display.redraw();
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
    Umd::Display.printf(0, line++, F("  %s"), fileName);
    Umd::Display.redraw();

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
