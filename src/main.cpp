#include <Arduino.h>
#include <SerialFlash.h>
#include <SoftwareSerial.h>         
#include <SerialCommand.h>                  // https://github.com/db-electronics/ArduinoSerialCommand

const int FlashChipSelect = PB0;            ///< Digital pin for flash chip CS pin
SerialFlashFile flashFile;                  ///< Serial flash file object
uint8_t sfID[5];                            ///< Serial flash file id
uint32_t sfSize;                            ///< Serial flash file size

SerialCommand SCmd;

void scmd_poke(void);

void setup() {
    // put your setup code here, to run once:

    pinMode(PB9, OUTPUT);
    SerialUSB.begin();

    if (!SerialFlash.begin(FlashChipSelect)) {
        //error("Unable to access SPI Flash chip");
        SerialUSB.println("Unable to access SPI Flash chip");
    }else{
        SerialFlash.readID(sfID);
        sfSize = SerialFlash.capacity(sfID);
    }
    //register callbacks for SerialCommand related to the cartridge
    SCmd.addCommand("poke", scmd_poke);
}

void loop() {

    // listen for commands
    SCmd.readSerial();

    // put your main code here, to run repeatedly:
    SerialUSB.println("digitalWrite()");
    digitalWrite(PB9, HIGH);   // turn the LED on (HIGH is the voltage level)
    digitalWrite(PB9, LOW);
    digitalWrite(PB9, HIGH);
    digitalWrite(PB9, LOW);
    delay(500); 

    // try to touch the GPIO port directly
    SerialUSB.println("direct io manipulation");
    GPIOB->ODR |= (1<<9);
    GPIOB->ODR &= ~(1<<9);
    GPIOB->ODR |= (1<<9);
    GPIOB->ODR &= ~(1<<9);
    delay(500);

}

void scmd_poke(void)
{
    char *arg;
    uint32_t address, value;
    
    // this is the address to poke
    arg = SCmd.next();
    address = (uint32_t)strtoul(arg, (char**)0, 0);

    value = *(__IO uint32_t *)(address);
    SerialUSB.print(value, HEX);
}