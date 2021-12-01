#include <Arduino.h>
#include <SerialFlash.h>
#include <SoftwareSerial.h>         
#include <SerialCommand.h>                  // https://github.com/db-electronics/ArduinoSerialCommand
#include <usbd_if.h>

const int FlashChipSelect = PB0;            ///< Digital pin for flash chip CS pin
SerialFlashFile flashFile;                  ///< Serial flash file object
uint8_t sfID[5];                            ///< Serial flash file id
uint32_t sfSize;                            ///< Serial flash file size

SerialCommand SCmd;

//#define USBD_ATTACH_PIN         PA12 //USB_OTG_FS_DP
//#define USBD_ATTACH_LEVEL       HIGH

void scmd_poke(void);

void setup() {
    // put your setup code here, to run once:
    SerialUSB.begin();
    SerialUSB.end();
    //USBD_reenumerate();
    pinMode(PA12, OUTPUT);
    digitalWrite(PA12, LOW);
    delay(10);
    SerialUSB.begin();

    pinMode(PA6, OUTPUT);
    pinMode(PA7, OUTPUT);
    digitalWrite(PA6, HIGH);
    digitalWrite(PA7, HIGH);
    digitalWrite(PA6, LOW);
    digitalWrite(PA7, LOW);
    digitalWrite(PA6, HIGH);
    digitalWrite(PA7, HIGH);

    // if (!SerialFlash.begin(FlashChipSelect)) {
    //     //error("Unable to access SPI Flash chip");
    //     SerialUSB.println("Unable to access SPI Flash chip");
    // }else{
    //     SerialFlash.readID(sfID);
    //     sfSize = SerialFlash.capacity(sfID);
    // }
    //register callbacks for SerialCommand related to the cartridge
    SCmd.addCommand("poke", scmd_poke);
}

void loop() {

    // listen for commands
    SCmd.readSerial();

    digitalWrite(PA6, HIGH);
    digitalWrite(PA6, LOW);
    digitalWrite(PA6, HIGH);
    GPIOA->ODR |= (1<<7);
    GPIOA->ODR &= ~(1<<7);
    GPIOA->ODR |= (1<<7);
    delay(50);
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