#include <Arduino.h>
#include <SoftwareSerial.h>         
#include <SerialCommand.h>                  // https://github.com/db-electronics/ArduinoSerialCommand
#include <USBSerial.h>
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include <crc.h>
#include <stm32f4xx_hal.h>
#include <stm32f4xx_hal_gpio.h>
#include <stm32f4xx_hal_crc.h>

SerialCommand SCmd;

void scmd_poke(void);

// I2C display
// https://randomnerdtutorials.com/guide-for-oled-display-with-arduino/
#define OLED_RESET -1 
#define OLED_SCREEN_WIDTH 128
#define OLED_SCREEN_HEIGHT 64
Adafruit_SSD1306 display(OLED_SCREEN_WIDTH, OLED_SCREEN_HEIGHT, &Wire, OLED_RESET);

// https://github.com/stm32duino/Arduino_Core_STM32/wiki/API#default-i2c-pins
const PinMap PinMap_I2C_SDA[] = {
  {PB_9,  I2C1, STM_PIN_DATA(STM_MODE_AF_OD, GPIO_NOPULL, GPIO_AF4_I2C1)},
  {NC,    NP,    0}
};
const PinMap PinMap_I2C_SCL[] = {
  {PB_8,  I2C1, STM_PIN_DATA(STM_MODE_AF_OD, GPIO_NOPULL, GPIO_AF4_I2C1)},
  {NC,    NP,    0}
};

// HAL Code in here? https://www.stm32duino.com/viewtopic.php?t=82

void setup() {

  MX_CRC_Init();

  // setup USB serial
  // https://primalcortex.wordpress.com/2020/10/11/stm32-blue-pill-board-arduino-core-and-usb-serial-output-on-platformio/
  SerialUSB.begin(460800);  // no need for a parameter here maybe?
  pinMode(PA10, OUTPUT);
  digitalWrite(PA10, LOW);
  delay(10);
  digitalWrite(PA10, HIGH);
  SerialUSB.println(F("Hello, World!"));

  // setup I2C
  // https://github.com/stm32duino/Arduino_Core_STM32/wiki/API#i2c
  // Wire.setSCL(PB8);
  // Wire.setSDA(PB9);
  Wire.setClock(100000);
  Wire.begin();
  Wire.setClock(100000);
  // Wire.beginTransmission(4);
  // Wire.write(0x55);
  // Wire.endTransmission();

  // C:\Users\rrichard\.platformio\packages\framework-arduinoststm32\variants\STM32F4xx\F407V(E-G)T_F417V(E-G)T\PeripheralPins.c
  pinMode(PB0, OUTPUT);
  pinMode(PB7, OUTPUT);

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
  display.setCursor(0,0);             // Start at top-left corner
  display.println(F("Hello, world!"));
  display.display();

  // set PB7 as output
  // https://controllerstech.com/stm32-gpio-output-config-using-registers/
  // GPIOB->MODER |= (1<<14); // output mode
  // GPIOB->OTYPER &= ~(1<<7); // push pull mode
  // GPIOB->OSPEEDR |= (1<<15); // fast speed
  // GPIOB->PUPDR &= ~(3<<7); // no pull-up or pull-down

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  GPIO_InitStruct.Pin = GPIO_PIN_7;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  GPIOB->BSRR |= (1<<7); // set the bit
  GPIOB->BSRR |= (1<<(7+16)); // reset the bit

    //register callbacks for SerialCommand related to the cartridge
    SCmd.addCommand("poke", scmd_poke);

}

void loop() {

  // delay(100);
  // digitalWrite(PB0, LOW);

  // delay(100);
  // digitalWrite(PB0, HIGH);
  
  // // SerialUSB.println(F("boo-urns!"));
  // GPIOB->BSRR |= (1<<7);      // set the bit
  // GPIOB->BSRR |= (1<<(7+16)); // reset the bit
  // GPIOB->BSRR |= (1<<7);      // set the bit
  // GPIOB->BSRR |= (1<<(7+16)); // reset the bit
  // GPIOB->BSRR |= (1<<7);      // set the bit
  // GPIOB->BSRR |= (1<<(7+16)); // reset the bit

  byte error, address;
  int nDevices;
  SerialUSB.println("Scanning...");
  nDevices = 0;
  for(address = 1; address < 127; address++ ) {
    Wire.beginTransmission(address);
    error = Wire.endTransmission();
    if (error == 0) {
      display.setCursor(0,0); 
      SerialUSB.print("I2C device found at address 0x");
      display.print(F("I2C device found at address 0x"));
      if (address<16) {
        SerialUSB.print("0");
        display.print("0");
      }
      SerialUSB.println(address,HEX);
      display.println(address,HEX);
      display.display();
      nDevices++;
    }
    else if (error==4) {
      SerialUSB.print("Unknow error at address 0x");
      if (address<16) {
        SerialUSB.print("0");
      }
      SerialUSB.println(address,HEX);
    }    
  }
  if (nDevices == 0) {
    SerialUSB.println("No I2C devices found\n");
  }
  else {
    SerialUSB.println("done\n");
  }
  delay(5000); 
}


void scmd_poke(void)
{
    char *arg;
    uint32_t address, value;
    
    // this is the address to poke
    // arg = SCmd.next();
    // address = (uint32_t)strtoul(arg, (char**)0, 0);

    // value = *(__IO uint32_t *)(address);
    // SerialUSB.print(value, HEX);
    __HAL_CRC_DR_RESET(&hcrc1);
}