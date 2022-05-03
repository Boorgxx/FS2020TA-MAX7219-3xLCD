

#include <LEDDisplayDriver.h>

// This module is tested with a TM1637 module with 4 digits
// Manual for library: http://lygte-info.dk/project/DisplayDriver%20UK.html
// By HKJ from lygte-info.dk

#ifndef _TM1637_
#error "_TM1637_ must be defined in LEDDisplayDriver.h for this sketch to work"
#endif

//*************************************************************************
// Define the pins used for the display connection
#define SDA_PIN A0
#define SCL_PIN A1


LEDDisplayDriver display(SDA_PIN, SCL_PIN);

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

// Split the display in two with a flashing colon (or point on some displays) between.
  for (int i=0;i<2000;i+=3) {
    display.showNum2LeftLZ(i/60);
    display.showNum2RightLZ(i%60);
    display.setDp((i%100)<50?1:255);
    delay(10);
  }
  display.removeDp();
}