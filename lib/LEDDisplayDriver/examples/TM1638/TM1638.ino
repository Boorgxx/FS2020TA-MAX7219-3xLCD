

#include <LEDDisplayDriver.h>

// This module is tested with a TM1638 module that is called LED&KEY with 8 leds, 8 digits and 8 keys.
// Manual for library: http://lygte-info.dk/project/DisplayDriver%20UK.html
// By HKJ from lygte-info.dk

#ifndef _TM1638_
#error "_TM1638_ must be defined in LEDDisplayDriver.h for this sketch to work"
#endif

//*************************************************************************
// Define the pins used for the display connection
#define DIO_PIN A0
#define CLK_PIN A1
#define STB_PIN A2


LEDDisplayDriver display(DIO_PIN, CLK_PIN, STB_PIN, true, 8);

byte mode = 0;

void setup() {
}

byte arduino[] = {digitA, digitR, digitD, digitu, digiti, digitN, digitO, digitEmpty};

void loop() {

  // Display millis in different ways depending on mode.
  switch (mode) {
    case 0 :  // As a integer
      display.showNum((long)millis());
      break;
    case 1 :  // As seconds with decimals
      display.showNum2decimals(millis() / 1000.0);
      break;
    case 2 :  // In hex
      display.showHex((long)millis());
      break;
    case 3: // In hours minutes seconds
      {
        unsigned long m = millis() / 1000;
        byte sec = m % 60; m /= 60;
        byte min = m % 60; m /= 60;
        byte hour = m;
        display.showNum2LeftLZ(hour);
        display.showDigit(digitEmpty, 2);
        display.showNum2CenterLZ(min);
        display.showDigit(digitEmpty, 5);
        display.showNum2RightLZ(sec);
      }
      break;
    case 4: // Show some bytes on the segments
      display.showDigits(arduino, 0, 8);
      break;
  }

  // Read the keys and select mode
  switch (display.readKeyIndexOnce()) {
    case KEY_S1: mode = 0; break;
    case KEY_S2: mode = 1; break;
    case KEY_S3: mode = 2; break;
    case KEY_S4: mode = 3; break;
    case KEY_S5: mode = 4; break;
    case KEY_S6: break;
    case KEY_S7: break;
    case KEY_S8: break;
  }

  // Set led according to mode
  display.showIndicators(1 << (mode * 2));

}