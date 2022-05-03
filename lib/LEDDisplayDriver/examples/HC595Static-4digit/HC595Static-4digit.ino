

#include <LEDDisplayDriver.h>

// This module is tested with a HC595 static module with 4 digits, i.e. four HC595
// Manual for library: http://lygte-info.dk/project/DisplayDriver%20UK.html
// By HKJ from lygte-info.dk

#ifndef _HC595_STATIC_
#error "_HC595_STATIC_ must be defined in LEDDisplayDriver.h for this sketch to work"
#endif

//*************************************************************************
// Define the pins used for the display connection
#define SDI_PIN A0
#define SCLK_PIN A1
#define LOAD_PIN A2


LEDDisplayDriver display(SDI_PIN, SCLK_PIN,LOAD_PIN);

void setup() {
}

byte helo[]={digitH,digitE,digitL,digitO};

void loop() {

// Show some bytes
  display.showDigits(helo,0,4);
  delay(3000);

// A normal number
  display.showNum(1234);
  delay(3000);

// Can also be shown in hex
  display.showHex(1234);
  delay(3000);

// Floating point with a comma
  display.showNum(4.456);
  delay(3000);

// Another floating point with a comma placed different
  display.showNum(22.34);
  delay(3000);

// Split the display in two with a flashing point between.
  for (int i=0;i<2000;i+=3) {
    display.showNum2LeftLZ(i/60);
    display.showNum2RightLZ(i%60);
    display.setDp((i%50)<25?1:255);
    delay(10);
  }
  display.removeDp();

}