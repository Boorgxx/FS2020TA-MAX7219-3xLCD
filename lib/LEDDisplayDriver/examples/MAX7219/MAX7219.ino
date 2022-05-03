

#include <LEDDisplayDriver.h>

// This module is tested with a MAX7219 module with 8 digits
// Manual for library: http://lygte-info.dk/project/DisplayDriver%20UK.html
// By HKJ from lygte-info.dk

#ifndef _MAX7219_
#error "_MAX7219_ must be defined in LEDDisplayDriver.h for this sketch to work"
#endif

//*************************************************************************
// Define the pins used for the display connection
#define DIN_PIN A0
#define CLK_PIN A1
#define CS_PIN A2

LEDDisplayDriver display(DIN_PIN, CLK_PIN, CS_PIN, true, 8);	// With 8 digits

void setup() {
}

byte hello[] = {digitH, digitE, digitL, digitL, digitO, digitEmpty, digitEmpty, digitEmpty};

void loop() {

  // Say hello
  display.showDigits(hello, 0, 8);
  delay(3000);

  // A number
  display.showNum(1234);
  delay(3000);

  // In hex
  display.showHex(1234);
  delay(3000);

  // Show number on part of display
  display.clear();
  display.showNum(4.1234567, 2, 6);
  delay(3000);

  // Use the full display for this number
  display.showNum(22.123456);
  delay(3000);

  // Show some text and a counting mumber
  display.clear();
  display.showDigit(digitT, 0);
  display.showDigit(digiti, 1);
  unsigned long t = millis();
  while (millis() < t + 15000) {
    display.showNum((millis() - t) / 1000.0, 4, 4);
  }

}