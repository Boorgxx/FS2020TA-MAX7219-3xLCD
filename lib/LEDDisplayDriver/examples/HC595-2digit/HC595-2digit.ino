

#include <LEDDisplayDriver.h>

// This module is tested with a HC595 module with 2 digits
// Manual for library: http://lygte-info.dk/project/DisplayDriver%20UK.html
// By HKJ from lygte-info.dk

#ifndef _HC595A_
#error "_HC595A_ must be defined in LEDDisplayDriver.h for this sketch to work"
#endif

// Demand use of timer intr1 because it is supported on both Mega328 and Mega32U4
#ifndef _USE_INTR1_
#error "_USE_INTR1_ must be defined in LEDDisplayDriver.h for this sketch to work"
#endif


//*************************************************************************
// Define the pins used for the display connection
#define DAT_PIN A0
#define SCLK_PIN A1
#define RCLK_PIN A2


LEDDisplayDriver display(DAT_PIN, SCLK_PIN, RCLK_PIN, false, 2);

void setup() {
}


// The interrupt handle will be inserted here:
DISPLAY_INTR(display)

void loop() {

  // Say Hi
  display.showDigit(digitH, 0);
  display.showDigit(SEG_C, 1);
  delay(3000);

  // Count in decimal
  for (int i = 99; i >= 0; i--) {
    display.showNum(i);
    delay(100);
  }
  
  // Done with counting
  display.showMinus(0, 2);
  delay(1000);
  
  // Count in hex
  for (int i = 0xa0; i < 0x100; i++) {
    display.showHex(i);
    delay(100);
  }
}