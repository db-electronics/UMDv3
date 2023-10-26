#include <Arduino.h>
#include <SoftwareSerial.h>         
#include <SerialCommand.h>                  // https://github.com/db-electronics/ArduinoSerialCommand
#include <USBSerial.h>
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include <stm32f4xx_hal.h>
#include <stm32f4xx_hal_gpio.h>

#include "umdBoardDefinitions.h"
#include "i2cScanner.h"
#include "mcp23008.h"
#include "umdDisplay.h"

SerialCommand SCmd;
MCP23008 onboardMCP23008;
MCP23008 adapterMCP23008;
UMDDisplay umdDisplay;

void scmdScanI2C(void);
void inputInterrupt(void);
uint8_t inputs;
bool newInputs;

// I2C display
// https://randomnerdtutorials.com/guide-for-oled-display-with-arduino/

Adafruit_SSD1306 display(OLED_SCREEN_WIDTH, OLED_SCREEN_HEIGHT, &Wire, OLED_RESET);

// HAL Code in here? https://www.stm32duino.com/viewtopic.php?t=82

void setup() {

  // setup USB serial
  // https://primalcortex.wordpress.com/2020/10/11/stm32-blue-pill-board-arduino-core-and-usb-serial-output-on-platformio/
  SerialUSB.begin(460800);  // no need for a parameter here maybe?
  pinMode(PA10, OUTPUT);
  digitalWrite(PA10, LOW);
  delay(10);
  digitalWrite(PA10, HIGH);
  SerialUSB.println(F("UMDv3"));

  // enable master clock output on MCo1 PA8
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  GPIO_InitStruct.Pin = GPIO_PIN_8;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF0_MCO;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
  HAL_RCC_MCOConfig(RCC_MCO1, RCC_MCO1SOURCE_HSE, RCC_MCODIV_4);

  // setup I2C
  // https://github.com/stm32duino/Arduino_Core_STM32/wiki/API#i2c
  // Wire.setSCL(PB8);
  // Wire.setSDA(PB9);
  Wire.begin();

  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3c)) { 
    SerialUSB.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }

  display.display();
  delay(2000); // Pause for 2 seconds
  display.clearDisplay();
  display.setTextSize(1);             // Normal 1:1 pixel scale
  display.setTextColor(WHITE);        // Draw white text
  display.setCursor(0, OLED_LINE_NUMBER(0));
  display.println(F("UMDv3 initiliazing..."));
  display.setCursor(0, OLED_LINE_NUMBER(1));
  display.display();

  delay(1000);
  umdDisplay.begin(&display);
  umdDisplay.clear();
  umdDisplay.print("/UMDv3/", 0);
  umdDisplay.redraw();

  // setup onboard mcp23008, GP6 and GP7 LED outputs
  umdDisplay.print("init MCP23008", 1, 2);
  umdDisplay.redraw();

  if(!onboardMCP23008.begin(UMD_BOARD_MCP23008_ADDRESS)){
    SerialUSB.println(F("onboard MCP23008 error"));
    //display.println(F("onboard MCP23008 error"));
    for(;;); // Don't proceed, loop forever
  }


  onboardMCP23008.pinMode(UMD_BOARD_LEDS, OUTPUT);
  onboardMCP23008.setPullUpResistors(UMD_BOARD_PUSHBUTTONS, true);
  onboardMCP23008.setDefaultValue(UMD_BOARD_PUSHBUTTONS, HIGH);
  onboardMCP23008.setInterruptControl(UMD_BOARD_PUSHBUTTONS, onboardMCP23008.PREVIOUS);
  onboardMCP23008.setInterruptOnChange(UMD_BOARD_PUSHBUTTONS, true);
  onboardMCP23008.setInterruptEnable(UMD_BOARD_PUSHBUTTONS, true);
  onboardMCP23008.digitalWrite(UMD_BOARD_LEDS, LOW);
  newInputs = false;
  inputs = onboardMCP23008.readGPIO();
  // interrupt line tied to PD1
  pinMode(UMD_MCP23008_INTERRUPT_PIN, INPUT);
  attachInterrupt(digitalPinToInterrupt(UMD_MCP23008_INTERRUPT_PIN), inputInterrupt, FALLING);

  // setup adapter mcp23008, read adapter id
  if(!adapterMCP23008.begin(UMD_ADAPTER_MCP23008_ADDRESS)){
    SerialUSB.println(F("adapter MCP23008 error"));
    for(;;); // Don't proceed, loop forever
  }

  adapterMCP23008.pinMode(0xFF, INPUT);
  uint8_t adapterId = adapterMCP23008.readGPIO();
  umdDisplay.print("adapter id = 1", 2, 2);
  umdDisplay.print("a really long string for scrolling",3,0);
  delay(2000);

  // C:\Users\rrichard\.platformio\packages\framework-arduinoststm32\variants\STM32F4xx\F407V(E-G)T_F417V(E-G)T\PeripheralPins.c
  pinMode(PB0, OUTPUT);
  pinMode(PB7, OUTPUT);

  // set PB7 as output
  // https://controllerstech.com/stm32-gpio-output-config-using-registers/
  // GPIOB->MODER |= (1<<14); // output mode
  // GPIOB->OTYPER &= ~(1<<7); // push pull mode
  // GPIOB->OSPEEDR |= (1<<15); // fast speed
  // GPIOB->PUPDR &= ~(3<<7); // no pull-up or pull-down


  GPIO_InitStruct.Pin = GPIO_PIN_7;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  GPIOB->BSRR |= (1<<7); // set the bit
  GPIOB->BSRR |= (1<<(7+16)); // reset the bit

  //register callbacks for SerialCommand related to the cartridge
  SCmd.addCommand("scani2c", scmdScanI2C);
}

void loop() {

  onboardMCP23008.tooglePins(MCP23008_GP7 | MCP23008_GP6);

  if(newInputs)
  {
    newInputs = false;
    inputs = onboardMCP23008.readInterruptCapture();
    SerialUSB.print(F("inputs = 0x"));
    SerialUSB.println(inputs, HEX);
  }

  umdDisplay.scrollLine(2, 1);
  umdDisplay.scrollLine(3, -1);
  umdDisplay.redraw();
  delay(250);
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
      SerialUSB.println(address,HEX);
    }

}

void inputInterrupt(void)
{
  newInputs = true;
}