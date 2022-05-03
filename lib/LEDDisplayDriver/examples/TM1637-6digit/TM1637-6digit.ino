

#include <LEDDisplayDriver.h>

// This module is tested with a TM1637 module with 6 digits and 6 keys
// Manual for library: http://lygte-info.dk/project/DisplayDriver%20UK.html
// By HKJ from lygte-info.dk

#ifndef _TM1637_
#error "_TM1637_ must be defined in LEDDisplayDriver.h for this sketch to work"
#endif

//*************************************************************************
// Define the pins used for the display connection
#define SDA_PIN A0
#define SCL_PIN A1


LEDDisplayDriver display(SDA_PIN, SCL_PIN, true, 6);

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
        display.showNum2CenterLZ(min);
        display.showNum2RightLZ(sec);
      }
      break;
    case 4: // Show some bytes on the segments
      display.showDigits(arduino, 0, 6);
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
  }


}