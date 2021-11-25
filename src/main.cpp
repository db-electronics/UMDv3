#include <Arduino.h>
#include <SerialFlash.h>

const int FlashChipSelect = 20;             ///< Digital pin for flash chip CS pin
SerialFlashFile flashFile;                  ///< Serial flash file object
uint8_t sfID[5];                            ///< Serial flash file id
uint32_t sfSize;                            ///< Serial flash file size

void setup() {
  // put your setup code here, to run once:

  pinMode(PB9, OUTPUT);
  SerialUSB.begin();

  if (!SerialFlash.begin(FlashChipSelect)) {
      //error("Unable to access SPI Flash chip");
  }else{
      SerialFlash.readID(sfID);
      sfSize = SerialFlash.capacity(sfID);
  }
}

void loop() {
  // put your main code here, to run repeatedly:
    digitalWrite(PB9, HIGH);   // turn the LED on (HIGH is the voltage level)
    delay(1000);                       // wait for a second
    digitalWrite(PB9, LOW);    // turn the LED off by making the voltage LOW
    delay(1000); 

    SerialUSB.println("hello");
}