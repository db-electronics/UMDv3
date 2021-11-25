#include <Arduino.h>
#include <SerialFlash.h>
#include <SoftwareSerial.h>         
#include <SerialCommand.h>                  // https://github.com/db-electronics/ArduinoSerialCommand

const int FlashChipSelect = PB0;            ///< Digital pin for flash chip CS pin
SerialFlashFile flashFile;                  ///< Serial flash file object
uint8_t sfID[5];                            ///< Serial flash file id
uint32_t sfSize;                            ///< Serial flash file size

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
}

void loop() {
  // put your main code here, to run repeatedly:
    SerialUSB.println("digitalWrite()");
    digitalWrite(PB9, HIGH);   // turn the LED on (HIGH is the voltage level)
    digitalWrite(PB9, LOW);
    digitalWrite(PB9, HIGH);
    digitalWrite(PB9, LOW);
    delay(1000); 

    // try to touch the GPIO port directly
    SerialUSB.println("direct io manipulation");
    GPIOB->ODR |= (1<<9);
    GPIOB->ODR &= ~(1<<9);
    GPIOB->ODR |= (1<<9);
    GPIOB->ODR &= ~(1<<9);
    delay(1000);

    SerialFlash.createErasable
}