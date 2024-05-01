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

#include <sstream>

#include "Umd.h"
#include "cartridges/Cartridge.h"
#include "config/RemapPins.h"
#include "services/I2cScanner.h"
#include "services/Crc32Calculator.h"

using umd::Key;
using cartridges::Cartridge;
using umd::Cart::CartState;

#ifndef SD_DETECT_PIN
#define SD_DETECT_PIN PD0
#endif

#define SD_CLK_DIV  8

//SdFatFs fatFs;
File sdFile;
SerialCommand SCmd;

// SD stuff
int verifySdCard();
int verifySdCardSystemSetup(const char* systemName);

void scmdScanI2C(void);

// MARK: Setup
void setup()
{
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

    if (!umd::Ux::Display.Init())
    {
        SerialUSB.println(F("display init failure"));
        while (1);
    }

    // enable the title zone
    umd::Ux::Display.SetZoneVisibility(UMDDisplay::ZONE_TITLE, true);
    umd::Ux::Display.SetZoneVisibility(UMDDisplay::ZONE_STATUS, true);

    // sd card
    umd::Ux::Display.Printf(UMDDisplay::ZONE_TITLE, F("initializing..."));
    umd::Ux::Display.Printf(UMDDisplay::ZONE_WINDOW, F("-sd card"));
    umd::Ux::Display.Redraw();

    // SD.setDx(PC8, PC9, PC10, PC11);
    // SD.setCMD(PD2);
    // SD.setCK(PC12);
    if (!SD.begin(SD_DETECT_PIN, PC8, PC9, PC10, PC11, PD2, PC12))
    {
        umd::Ux::Display.Printf(UMDDisplay::ZONE_STATUS, F("err: no sd card"));
        umd::Ux::Display.Redraw();
        while (1);
    }

    // reduce the SDIO clock, seems more stable like this
    SDIO->CLKCR |= SD_CLK_DIV; 

    // check that the SD cart is properly formatted for UMDv3
    int sdVerify = verifySdCard();
    if(sdVerify != 0)
    {
        umd::Ux::Display.Printf(UMDDisplay::ZONE_STATUS, F("err: no UMD folder"));
        umd::Ux::Display.Redraw();
        while (1);
    }

    // setup onboard mcp23008, GP6 and GP7 LED outputs
    umd::Ux::Display.Printf(UMDDisplay::ZONE_WINDOW, F("-mcp23008 io expander"));
    umd::Ux::Display.Redraw();

    if (!umd::IoExpander.begin(umd::Config::MCP23008_BOARD_ADDRESS))
    {
        umd::Ux::Display.Printf(UMDDisplay::ZONE_STATUS, F("err: not found"));
        umd::Ux::Display.Redraw();
        while (1);
    }

    umd::IoExpander.pinMode(umd::Config::MCP23008_LEDS, OUTPUT);
    umd::IoExpander.setPullUpResistors(umd::Config::MCP23008_PUSHBUTTONS, true);
    umd::IoExpander.setInterruptOnChange(umd::Config::MCP23008_PUSHBUTTONS, true);
    umd::IoExpander.setInterruptControl(umd::Config::MCP23008_PUSHBUTTONS, umd::IoExpander.PREVIOUS);
    umd::IoExpander.setInterruptEnable(umd::Config::MCP23008_PUSHBUTTONS, true);
    umd::IoExpander.digitalWrite(umd::Config::MCP23008_LEDS, LOW);

    // setup adapter mcp23008, read adapter id
    umd::Ux::Display.Printf(UMDDisplay::ZONE_WINDOW, F("-mcp23008 adapter"));
    umd::Ux::Display.Redraw();
    if (!umd::Cart::IoExpander.begin(umd::Config::MCP23008_ADAPTER_ADDRESS))
    {
        umd::Ux::Display.Printf(UMDDisplay::ZONE_STATUS, F("err: not found"));
        umd::Ux::Display.Redraw();
        while (1);
    }

    // get the adapter id, use it to determine the cartridge type
    umd::Cart::IoExpander.pinMode(0xFF, INPUT);
    uint8_t adapterId = umd::Cart::IoExpander.readGPIO();
    umd::Cart::pCartridge = umd::Cart::Factory.MakeCartridge(adapterId);
    if (umd::Cart::pCartridge == nullptr)
    {
        umd::Ux::Display.Printf(UMDDisplay::ZONE_STATUS, F("err: unknown adapter"));
        umd::Ux::Display.Redraw();
        while (1);
    }

    //umd::Ux::Display.printf(0, line++, F("  adapter id = %d"), adapterId);
    auto systemName = umd::Cart::pCartridge->GetSystemName();
    umd::Ux::Display.Printf(UMDDisplay::ZONE_WINDOW, F("-%s"), systemName.c_str());
    umd::Ux::Display.Redraw();

    // Init game identifier
    if(!umd::pGameIdentifier->Init(umd::Cart::pCartridge->GetSystemBaseFilePath()))
    {
        umd::Ux::Display.Printf(UMDDisplay::ZONE_STATUS, F("err: no sys db file"));
        umd::Ux::Display.Redraw();
        while (1);
    
    };

    // check if the sdcard has a _db.txt file containing rom checksums for this system
    // sdVerify = verifySdCardSystemSetup(systemName.c_str());
    // if(sdVerify != 0)
    // {
    //     umd::Ux::Display.Printf(UMDDisplay::ZONE_STATUS, F("err: no sys db file"));
    //     umd::Ux::Display.Redraw();
    //     while (1);
    // }

    // register callbacks for SerialCommand related to the cartridge
    SCmd.addCommand("scani2c", scmdScanI2C);

    // MARK: Init Success
    umd::Ux::Display.Printf(UMDDisplay::ZONE_STATUS, F("init success"));
    umd::Ux::Display.Redraw();
    delay(2000);

    umd::Ux::Display.ClearZone(UMDDisplay::ZONE_TITLE);
    umd::Ux::Display.ClearZone(UMDDisplay::ZONE_WINDOW);
    umd::Ux::Display.ClearZone(UMDDisplay::ZONE_STATUS);

    // get the supported memory types for this cartridge
    umd::Cart::MemoryNames = umd::Cart::pCartridge->GetMemoryNames();
    
    umd::Ux::Display.Redraw();

    umd::Ux::State = umd::Ux::UX_MAIN_MENU;
    umd::Ux::UserInputState = umd::Ux::UX_INPUT_INIT;
    umd::Cart::State = CartState::IDLE;

    std::fill(std::begin(umd::CartridgeData), std::end(umd::CartridgeData), 0xAA);
}

//MARK: Main loop
void loop()
{
    // Reminder: when debugging ticks isn't accurate at all and SD card is more wonky
    static uint32_t currentTicks=0, previousTicks, dasTicks;
    uint8_t selectedItemIndex;
    //Cartridge::MemoryType selectedMemory;
    uint32_t totalBytes;

    // get the ticks
    previousTicks = currentTicks;
    currentTicks = HAL_GetTick();

    // process inputs
    uint8_t inputs = umd::IoExpander.readGPIO();
    umd::Ux::Keys.Process(inputs, currentTicks);
    
    switch(umd::Ux::UserInputState)
    {
        case umd::Ux::UX_INPUT_WAIT_FOR_PRESSED:
            // user pressed down
            if(umd::Ux::Keys.Down >= Key::Pressed)
            {
                umd::Ux::Display.UpdateCursorItemPosition(1);
                umd::Ux::Display.ResetScrollX();
                umd::Ux::UserInputState = umd::Ux::UX_INPUT_WAIT_FOR_RELEASED;
                dasTicks = currentTicks;
            }
            // user pressed up
            else if (umd::Ux::Keys.Up >= Key::Pressed)
            {
                umd::Ux::Display.UpdateCursorItemPosition(-1);
                umd::Ux::Display.ResetScrollX();
                umd::Ux::UserInputState = umd::Ux::UX_INPUT_WAIT_FOR_RELEASED;
                dasTicks = currentTicks;
            }
            // user pressed left
            else if (umd::Ux::Keys.Left >= Key::Pressed)
            {
                umd::Ux::Display.SetWindowItemScrollX(-1);
                umd::Ux::UserInputState = umd::Ux::UX_INPUT_WAIT_FOR_RELEASED;
                dasTicks = currentTicks;
            }
            // user pressed right
            else if (umd::Ux::Keys.Right >= Key::Pressed)
            {
                umd::Ux::Display.SetWindowItemScrollX(1);
                umd::Ux::UserInputState = umd::Ux::UX_INPUT_WAIT_FOR_RELEASED;
                dasTicks = currentTicks;
            }
            // user pressed ok
            else if (umd::Ux::Keys.Ok >= Key::Pressed){
                // find which item the user clicked on
                selectedItemIndex = umd::Ux::Display.GetSelectedItemIndex();
                switch(umd::Ux::State){
                    // MARK: Main Menu
                    case umd::Ux::UX_MAIN_MENU:
                        switch(selectedItemIndex)
                        {
                            // MARK: Identify
                            // these must match the index of the menu items currently displayed
                            // Identify the connected cartridge by calculating the CRC32 of the ROM and comparing against the database
                            case CartState::IDENTIFY:
                                // update state to IDENTIFY,
                                umd::Cart::State = CartState::IDENTIFY;
                                
                                // clear window and status
                                umd::Ux::Display.ClearZone(UMDDisplay::ZONE_WINDOW);
                                umd::Ux::Display.ClearZone(UMDDisplay::ZONE_STATUS);
                                umd::Ux::Display.Printf(UMDDisplay::ZONE_TITLE, F("UMDv3/%s/%s"), umd::Cart::pCartridge->GetSystemName().c_str(), "Id");

                                currentTicks = HAL_GetTick();
                                umd::OperationStartTime = currentTicks;
                                umd::Cart::pCartridge->ResetChecksumCalculator();
                                totalBytes = umd::Cart::pCartridge->GetCartridgeSize();
                                umd::CartridgeData.SetTransferSize(totalBytes);
                                umd::Ux::Display.SetProgressBarVisibility(true);

                                for(int addr = 0; addr < totalBytes; addr += umd::CartridgeData.Size())
                                {
                                    umd::Cart::pCartridge->Identify(addr, umd::CartridgeData, Cartridge::ReadOptions::CHECKSUM_CALCULATOR);
                                    if(HAL_GetTick() > currentTicks + umd::Config::PROGRESS_REFRESH_RATE_MS)
                                    {
                                        currentTicks = HAL_GetTick();
                                        umd::Ux::Display.UpdateProgressBar(addr, totalBytes);
                                        umd::Ux::Display.Redraw(); 
                                    }
                                }
                                umd::OperationTotalTime = HAL_GetTick() - umd::OperationStartTime;
                                umd::Ux::Display.SetProgressBarComplete(umd::OperationTotalTime);

                                // TODO get rid of metadata from cartridge and get the title instead
                                // seems better from an encapsulation perspective
                                umd::Cart::Metadata.clear();
                                umd::Cart::Metadata = umd::Cart::pCartridge->GetMetadata();
                                
                                umd::Ux::Display.NewWindow(umd::Cart::Metadata);
                                umd::Ux::Display.Printf(F("Size : %08X"), totalBytes);
                                umd::Ux::Display.Printf(F("CRC  : %08X"), umd::Cart::pCartridge->GetAccumulatedChecksum());
                                                                
                                // search for this game id in the database
                                umd::StringStream.clear();
                                umd::StringStream << std::hex << umd::Cart::pCartridge->GetAccumulatedChecksum();
                                if(umd::pGameIdentifier->GameExists(umd::StringStream.str())){
                                    // std::string gameName = umd::pGameIdentifier->GetGameName(ss.str());
                                    umd::Ux::Display.Printf(F("Game : %s"), umd::pGameIdentifier->GetGameName(umd::StringStream.str()).c_str());
                                    umd::Ux::Display.Redraw();
                                }

                                // all done, return to main menu
                                umd::Cart::State = CartState::IDLE;
                                umd::Ux::State = umd::Ux::UX_MAIN_MENU;
                                umd::Ux::UserInputState = umd::Ux::UX_INPUT_WAIT_FOR_RELEASED;
                                break;
                            // MARK: Select Read
                            case CartState::READ: 
                                // update state to READ and offer choice of memory to read from
                                umd::Cart::State = CartState::READ;
                                umd::Ux::Display.Printf(UMDDisplay::ZONE_TITLE, F("UMDv3/%s/%s"), umd::Cart::pCartridge->GetSystemName().c_str(), "Read");
                                umd::Ux::Display.NewWindow(umd::Cart::MemoryNames);
                                umd::Ux::State = umd::Ux::UX_SELECT_MEMORY;
                                umd::Ux::UserInputState = umd::Ux::UX_INPUT_WAIT_FOR_RELEASED;
                                break;
                            // MARK: Select Write
                            case CartState::WRITE:
                                // update state to WRITE and offer choice of memory to write to
                                umd::Cart::State = CartState::WRITE;
                                umd::Ux::Display.Printf(UMDDisplay::ZONE_TITLE, F("UMDv3/%s/%s"), umd::Cart::pCartridge->GetSystemName().c_str(), "Write");
                                umd::Ux::Display.NewWindow(umd::Cart::MemoryNames);
                                umd::Ux::State = umd::Ux::UX_SELECT_MEMORY;
                                umd::Ux::UserInputState = umd::Ux::UX_INPUT_WAIT_FOR_RELEASED;
                                break;
                            default:
                                umd::Cart::State = CartState::IDLE;
                                umd::Ux::UserInputState = umd::Ux::UX_INPUT_WAIT_FOR_RELEASED;
                                break;
                        }
                        break;
                    // MARK: Select Memory
                    case umd::Ux::UX_SELECT_MEMORY:
                        
                        switch(umd::Cart::State)
                        {
                            case CartState::READ:
                                // TODO need a filename
                                
                                // selected index indicates the memory to read from
                                // umd::OperationStartTime = HAL_GetTick();
                                // totalBytes = umd::Cart::pCartridge->GetCartridgeSize();
                                // umd::Cart::BatchSizeCalc.Init(umd::Cart::pCartridge->GetCartridgeSize(), umd::Config::BUFFER_SIZE_BYTES);

                                // // TODO show some progress here, Sonic 3D Blast takes 1473ms to identify
                                // for(int addr = 0; addr < totalBytes; addr += umd::Config::BUFFER_SIZE_BYTES)
                                // {
                                //     batchSize = umd::Cart::BatchSizeCalc.Next();
                                //     umd::Cart::pCartridge->ReadMemory(addr, umd::DataBuffer.data(), batchSize, selectedItemIndex, Cartridge::ReadOptions::CHECKSUM_CALCULATOR);
                                // }

                                umd::OperationTotalTime = HAL_GetTick() - umd::OperationStartTime;

                                // all done, return to main menu
                                umd::Cart::State = CartState::IDLE;
                                umd::Ux::State = umd::Ux::UX_MAIN_MENU;
                                umd::Ux::UserInputState = umd::Ux::UX_INPUT_WAIT_FOR_PRESSED;
                                break;
                            case CartState::WRITE:
                                // selected index indicates the memory to write to
                                
                                // all done, return to main menu
                                umd::Cart::State = CartState::IDLE;
                                umd::Ux::State = umd::Ux::UX_MAIN_MENU;
                                umd::Ux::UserInputState = umd::Ux::UX_INPUT_WAIT_FOR_PRESSED;
                                break;
                            default:
                                umd::Cart::State = CartState::IDLE;
                                umd::Ux::State = umd::Ux::UX_MAIN_MENU;
                                umd::Ux::UserInputState = umd::Ux::UX_INPUT_WAIT_FOR_PRESSED;
                                break;
                        }
                        break;
                    default:
                        // debounce and return to main menu
                        umd::Cart::State = CartState::IDLE;
                        umd::Ux::State = umd::Ux::UX_MAIN_MENU;
                        umd::Ux::UserInputState = umd::Ux::UX_INPUT_WAIT_FOR_RELEASED;
                        break;
                }
            }
            // user pressed back, always return to main menu
            else if (umd::Ux::Keys.Back >= Key::Pressed){
                umd::Ux::Display.ClearZone(UMDDisplay::ZONE_STATUS);
                umd::Ux::Display.Printf(UMDDisplay::ZONE_TITLE, F("UMDv3/%s"), umd::Cart::pCartridge->GetSystemName().c_str());
                umd::Ux::Display.SetCursorVisibility(true);
                umd::Ux::Display.SetProgressBarVisibility(false);
                umd::Ux::Display.SetCursorChar('>');
                umd::Ux::Display.ResetScrollX();
                umd::Ux::Display.NewWindow(umd::Ux::MAIN_MENU_ITEMS);
                umd::Ux::UserInputState = umd::Ux::UX_INPUT_WAIT_FOR_RELEASED;
            }
            break;
        case umd::Ux::UX_INPUT_WAIT_FOR_RELEASED:
            // release all keys before accepting a new input
            if(umd::Ux::Keys.Ok == Key::Off && 
                umd::Ux::Keys.Back == Key::Off && 
                umd::Ux::Keys.Up == Key::Off && 
                umd::Ux::Keys.Down == Key::Off && 
                umd::Ux::Keys.Left == Key::Off &&
                umd::Ux::Keys.Right == Key::Off)
            {
                umd::Ux::UserInputState = umd::Ux::UX_INPUT_WAIT_FOR_PRESSED;
            }
            // MARK: Delayed auto-shift
            if(umd::Ux::Keys.Up == Key::Held){
                if(currentTicks > dasTicks + umd::Config::DAS_REPEAT_RATE_MS){
                    umd::Ux::Display.UpdateCursorItemPosition(-1);
                    dasTicks = currentTicks;
                }
            }else if(umd::Ux::Keys.Down == Key::Held){
                if(currentTicks > dasTicks + umd::Config::DAS_REPEAT_RATE_MS){
                    umd::Ux::Display.UpdateCursorItemPosition(1);
                    dasTicks = currentTicks;
                }
            }else if(umd::Ux::Keys.Left == Key::Held){
                if(currentTicks > dasTicks + umd::Config::DAS_REPEAT_RATE_MS){
                    umd::Ux::Display.SetWindowItemScrollX(-1);
                    dasTicks = currentTicks;
                }
            }else if(umd::Ux::Keys.Right == Key::Held){
                if(currentTicks > dasTicks + umd::Config::DAS_REPEAT_RATE_MS){
                    umd::Ux::Display.SetWindowItemScrollX(1);
                    dasTicks = currentTicks;
                } 
            }
            break;
        case umd::Ux::UX_INPUT_INIT:
        default:
            umd::Ux::Display.ClearZone(UMDDisplay::ZONE_STATUS);
            umd::Ux::Display.Printf(UMDDisplay::ZONE_TITLE, F("UMDv3/%s"), umd::Cart::pCartridge->GetSystemName().c_str());
            umd::Ux::Display.SetCursorVisibility(true);
            umd::Ux::Display.SetProgressBarVisibility(false);
            umd::Ux::Display.SetCursorChar('>');
            umd::Ux::Display.ResetScrollX();
            // umd::Ux::Display.NewWindow(umd::Ux::MENU_WITH_30_ITEMS);
            // umd::Ux::Display.Printf(UMDDisplay::ZONE_STATUS, F("test many items"));
            // umd::Ux::Display.Printf(F("hello world %d"), 1); // add a new item at the end
            umd::Ux::Display.NewWindow(umd::Ux::MAIN_MENU_ITEMS);
            umd::Cart::State = CartState::IDLE;
            umd::Ux::UserInputState = umd::Ux::UX_INPUT_WAIT_FOR_PRESSED;
            break;
    }

    umd::Ux::Display.Redraw();
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
int verifySdCard()
{
    sdFile = SD.open("/UMD");
    //File file = SD.open("/UMD/Genesis/_db.txt");
    if(!sdFile)
    {
        return 1;
    }

    // auto fileName = sdFile.fullname();
    // umd::Ux::Display.printf(0, line++, F("  %s"), fileName);
    // umd::Ux::Display.redraw();
    sdFile.close();

    return 0;
}

int verifySdCardSystemSetup(const char* systemName)
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
