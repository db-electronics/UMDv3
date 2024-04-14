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

    if (!Umd::Ux::Display.begin())
    {
        SerialUSB.println(F("display init failure"));
        while (1);
    }

    // sd card
    Umd::Ux::Display.printf(0, line++, F("init SD card"));
    Umd::Ux::Display.redraw();
    // SD.setDx(PC8, PC9, PC10, PC11);
    // SD.setCMD(PD2);
    // SD.setCK(PC12);
    if (!SD.begin(SD_DETECT_PIN, PC8, PC9, PC10, PC11, PD2, PC12))
    {
        Umd::Ux::Display.printf(0, line, F("  no card"));
        Umd::Ux::Display.redraw();
        while (1);
    }

    // reduce the SDIO clock, seems more stable like this
    SDIO->CLKCR |= SD_CLK_DIV; 
    
    // check that the SD cart is properly formatted for UMDv3
    int sdVerify = verifySdCard(line);
    if(sdVerify != 0)
    {
        Umd::Ux::Display.printf(0, line, F("  SD error %d"), sdVerify);
        Umd::Ux::Display.redraw();
        while (1);
    }

    // setup onboard mcp23008, GP6 and GP7 LED outputs
    Umd::Ux::Display.printf(0, line++, F("init umd MCP23008"));
    Umd::Ux::Display.redraw();

    if (!Umd::IoExpander.begin(UMD_BOARD_MCP23008_ADDRESS))
    {
        Umd::Ux::Display.printf(0, line, F("  failed"));
        Umd::Ux::Display.redraw();
        while (1);
    }

    Umd::IoExpander.pinMode(UMD_BOARD_LEDS, OUTPUT);
    Umd::IoExpander.setPullUpResistors(UMD_BOARD_PUSHBUTTONS, true);
    Umd::IoExpander.setInterruptOnChange(UMD_BOARD_PUSHBUTTONS, true);
    Umd::IoExpander.setInterruptControl(UMD_BOARD_PUSHBUTTONS, Umd::IoExpander.PREVIOUS);
    Umd::IoExpander.setInterruptEnable(UMD_BOARD_PUSHBUTTONS, true);
    Umd::IoExpander.digitalWrite(UMD_BOARD_LEDS, LOW);

    // setup adapter mcp23008, read adapter id
    Umd::Ux::Display.printf(0, line++, F("init cart MCP23008"));
    Umd::Ux::Display.redraw();
    if (!Umd::Cart::IoExpander.begin(UMD_ADAPTER_MCP23008_ADDRESS))
    {
        Umd::Ux::Display.printf(0, line, F("  no adapter found"));
        Umd::Ux::Display.redraw();
        while (1);
    }

    Umd::Cart::IoExpander.pinMode(0xFF, INPUT);
    uint8_t adapterId = Umd::Cart::IoExpander.readGPIO();

    Umd::Cart::pCartridge = Umd::Cart::Factory.GetCart(adapterId, crc32Calculator);
    if (Umd::Cart::pCartridge == nullptr)
    {
        Umd::Ux::Display.printf(0, line++, F("  unknown adapter"));
        Umd::Ux::Display.redraw();
        while (1);
    }

    //Umd::Ux::Display.printf(0, line++, F("  adapter id = %d"), adapterId);
    auto systemName = Umd::Cart::pCartridge->GetSystemName();
    Umd::Ux::Display.printf(0, line++, F("  %s"), systemName);
    Umd::Ux::Display.redraw();

    // check if the sdcard has a _db.txt file containing rom checksums for this system
    sdVerify = verifySdCardSystemSetup(line, systemName);
    if(sdVerify != 0)
    {
        Umd::Ux::Display.printf(0, line, F("  SD error %d"), sdVerify);
        Umd::Ux::Display.redraw();
        while (1);
    }

    // register callbacks for SerialCommand related to the cartridge
    SCmd.addCommand("scani2c", scmdScanI2C);

    delay(2000);

    // setup the display with 2 layers, having the UMDv3/... static on the first line
    Umd::Ux::Display.clear();
    Umd::Ux::Display.setLayerLineLength(0, 1);
    Umd::Ux::Display.setLayerLineLength(1, UMD_DISPLAY_BUFFER_TOTAL_LINES);
    Umd::Ux::Display.printf(0, 0, F("UMDv3/%s"), systemName);

    // get the supported memory types for this cartridge
    Umd::Cart::MemoryNames = Umd::Cart::pCartridge->GetMemoryNames();

    Umd::Ux::Display.setCursorPosition(0, 0);
    Umd::Ux::Display.setCursorVisible(true);
    Umd::Ux::Display.setClockPosition(20, 7);
    Umd::Ux::Display.setClockVisible(false);
    
    Umd::Ux::Display.redraw();

    Umd::Ux::UserInputState = Umd::Ux::INIT_MAIN_MENU;
    Umd::Cart::State = Cartridge::IDLE;
}

//MARK: Main loop
void loop()
{
    // Reminder: when debugging ticks isn't accurate at all and SD card is more wonky
    static uint32_t currentTicks=0, previousTicks;

    // get the ticks
    previousTicks = currentTicks;
    currentTicks = HAL_GetTick();

    // process inputs
    uint8_t inputs = Umd::IoExpander.readGPIO();
    Umd::Ux::UserInput.process(inputs, currentTicks);

    switch(Umd::Ux::UserInputState)
    {
        case Umd::Ux::WAIT_FOR_INPUT:
            if(Umd::Ux::UserInput.Down >= Umd::Ux::UserInput.PRESSED)
            {
                Umd::Ux::Display.menuCursorUpdate(1, true);
                Umd::Ux::UserInputState = Umd::Ux::WAIT_FOR_RELEASE;
            }
            else if (Umd::Ux::UserInput.Up >= Umd::Ux::UserInput.PRESSED)
            {
                Umd::Ux::Display.menuCursorUpdate(-1, true);
                Umd::Ux::UserInputState = Umd::Ux::WAIT_FOR_RELEASE;
            }
            else if (Umd::Ux::UserInput.Left >= Umd::Ux::UserInput.PRESSED)
            {
                // TODO scroll line left
                Umd::Ux::UserInputState = Umd::Ux::WAIT_FOR_RELEASE;
            }
            else if (Umd::Ux::UserInput.Right >= Umd::Ux::UserInput.PRESSED)
            {
                // TODO scroll line right
                Umd::Ux::UserInputState = Umd::Ux::WAIT_FOR_RELEASE;
            }
            else if (Umd::Ux::UserInput.Ok >= Umd::Ux::UserInput.PRESSED){
                // what item is selected?
                int menuItemIndex = Umd::Ux::Display.menuCurrentItem();
                
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

                                Umd::Cart::BatchSizeCalc.Init(Umd::Cart::pCartridge->GetCartridgeSize(), Umd::BUFFER_SIZE_BYTES);
                                for(int addr = 0; addr < Umd::Cart::BatchSizeCalc.TotalBytes(); addr += Umd::BUFFER_SIZE_BYTES)
                                {
                                    Umd::Cart::pCartridge->Identify(addr, Umd::DataBuffer.bytes, Umd::Cart::BatchSizeCalc.Next(), Cartridge::ReadOptions::HW_CHECKSUM);
                                }

                                Umd::OperationTime = HAL_GetTick() - currentTicks;

                                // ask cartridge for some metadata about the ROM
                                Umd::Cart::Metadata.clear();
                                Umd::Cart::Metadata = Umd::Cart::pCartridge->GetMetadata();

                                Umd::Ux::UpdateDisplayPathAddressBar(Umd::Cart::pCartridge->GetSystemName(), "Id");
                                Umd::Ux::Display.showMenu(UMD_DISPLAY_LAYER_MENU, Umd::Cart::Metadata);
                                Umd::Ux::Display.AddMenuItem(F("Size : %08X"), Umd::Cart::BatchSizeCalc.TotalBytes());
                                Umd::Ux::Display.AddMenuItem(F("CRC32: %08X"), Umd::Cart::pCartridge->GetAccumulatedChecksum());

                                // TODO search for this crc32 in the database
                                Umd::Cart::State = Cartridge::IDLE;
                                Umd::Ux::UserInputState = Umd::Ux::WAIT_FOR_INPUT;
                                break;
                            case Cartridge::READ: 
                                // update state to READ and offer choice of memory to read from
                                Umd::Cart::State = Cartridge::READ;
                                Umd::Ux::UpdateDisplayPathAddressBar(Umd::Cart::pCartridge->GetSystemName(), "Read");
                                Umd::Ux::Display.showMenu(UMD_DISPLAY_LAYER_MENU, Umd::Cart::MemoryNames);
                                Umd::Ux::UserInputState = Umd::Ux::WAIT_FOR_RELEASE;
                                break;
                            case Cartridge::WRITE:
                                // update state to WRITE and offer choice of memory to write to
                                Umd::Cart::State = Cartridge::WRITE;
                                Umd::Ux::UpdateDisplayPathAddressBar(Umd::Cart::pCartridge->GetSystemName(), "Write");
                                Umd::Ux::Display.showMenu(UMD_DISPLAY_LAYER_MENU, Umd::Cart::MemoryNames);
                                Umd::Ux::UserInputState = Umd::Ux::WAIT_FOR_RELEASE;
                                break;
                            default:
                                Umd::Cart::State = Cartridge::IDLE;
                                Umd::Ux::UserInputState = Umd::Ux::WAIT_FOR_RELEASE;
                                break;
                        }
                        break;
                    case Cartridge::READ:
                        // keep on reading
                        // finished the read
                        Umd::Cart::State = Cartridge::IDLE;
                        Umd::Ux::UserInputState = Umd::Ux::WAIT_FOR_INPUT;
                        break;
                    case Cartridge::WRITE:
                        // keep on writing
                        // finished the write
                        Umd::Cart::State = Cartridge::IDLE;
                        Umd::Ux::UserInputState = Umd::Ux::WAIT_FOR_INPUT;
                        break;
                    default:
                        Umd::Cart::State = Cartridge::IDLE;
                        Umd::Ux::UserInputState = Umd::Ux::WAIT_FOR_INPUT;
                        break;
                }
            }
            else if (Umd::Ux::UserInput.Back >= Umd::Ux::UserInput.PRESSED){
                Umd::Ux::UpdateDisplayPathAddressBar(Umd::Cart::pCartridge->GetSystemName(), "");
                Umd::Ux::Display.showMenu(UMD_DISPLAY_LAYER_MENU, UMD_MENU_MAIN);
                Umd::Ux::UserInputState = Umd::Ux::WAIT_FOR_RELEASE;
            }
            break;
        case Umd::Ux::WAIT_FOR_RELEASE: // el-cheapo debounce
            if(Umd::Ux::UserInput.Ok == Umd::Ux::UserInput.OFF && Umd::Ux::UserInput.Back == Umd::Ux::UserInput.OFF && Umd::Ux::UserInput.Up == Umd::Ux::UserInput.OFF && Umd::Ux::UserInput.Down == Umd::Ux::UserInput.OFF)
            {
                Umd::Ux::UserInputState = Umd::Ux::WAIT_FOR_INPUT;
            }
            break;
        case Umd::Ux::INIT_MAIN_MENU:
        default:
            Umd::Ux::UpdateDisplayPathAddressBar(Umd::Cart::pCartridge->GetSystemName(), "");
            Umd::Ux::Display.showMenu(UMD_DISPLAY_LAYER_MENU, UMD_MENU_MAIN);
            Umd::Cart::State = Cartridge::IDLE;
            Umd::Ux::UserInputState = Umd::Ux::WAIT_FOR_INPUT;
            break;
    }

    Umd::Ux::Display.advanceClockAnimation();
    Umd::Ux::Display.redraw();
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
    Umd::Ux::Display.printf(0, line++, F("  %s"), fileName);
    Umd::Ux::Display.redraw();
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
    Umd::Ux::Display.printf(0, line++, F("  %s"), fileName);
    Umd::Ux::Display.redraw();

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
