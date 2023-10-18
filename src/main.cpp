#include <Arduino.h>
#include <SoftwareSerial.h>         
#include <SerialCommand.h>                  // https://github.com/db-electronics/ArduinoSerialCommand
#include <USBSerial.h>

SerialCommand SCmd;

void scmd_poke(void);

void setup() {
  pinMode(PB0, OUTPUT);
  pinMode(PB7, OUTPUT);

  // https://primalcortex.wordpress.com/2020/10/11/stm32-blue-pill-board-arduino-core-and-usb-serial-output-on-platformio/
  SerialUSB.begin(460800);  // no need for a parameter here maybe?
  pinMode(PA10, OUTPUT);
  digitalWrite(PA10, LOW);
  delay(5);
  digitalWrite(PA10, HIGH);

  // set PB7 as output
  // https://controllerstech.com/stm32-gpio-output-config-using-registers/
  // GPIOB->MODER |= (1<<14); // output mode
  // GPIOB->OTYPER &= ~(1<<7); // push pull mode
  // GPIOB->OSPEEDR |= (1<<15); // fast speed
  // GPIOB->PUPDR &= ~(3<<7); // no pull-up or pull-down
  GPIOB->BSRR |= (1<<7); // set the bit
  GPIOB->BSRR |= (1<<(7+16)); // reset the bit

  SerialUSB.println(F("Hello, World!"));

    //register callbacks for SerialCommand related to the cartridge
    SCmd.addCommand("poke", scmd_poke);

}

void loop() {

  delay(100);
  digitalWrite(PB0, LOW);

  delay(100);
  digitalWrite(PB0, HIGH);
  
  // SerialUSB.println(F("boo-urns!"));
  GPIOB->BSRR |= (1<<7);      // set the bit
  GPIOB->BSRR |= (1<<(7+16)); // reset the bit
  GPIOB->BSRR |= (1<<7);      // set the bit
  GPIOB->BSRR |= (1<<(7+16)); // reset the bit
  GPIOB->BSRR |= (1<<7);      // set the bit
  GPIOB->BSRR |= (1<<(7+16)); // reset the bit
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
}