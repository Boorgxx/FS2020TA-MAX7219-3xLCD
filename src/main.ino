//* INFO V.1.1. Boorgxx 3LCD MAX2119 FS2020TA - duben2022

/**-----------------------------------------------------------------------------------------------------------------------
  DEFINICE DISPLAY
  This module is tested with a MAX7219 module with 8 digits
  Manual for library: http://lygte-info.dk/project/DisplayDriver%20UK.html
  By HKJ from lygte-info.dk
  For MAX7219 edit library *.h file (uncoment MAX..)
 *-----------------------------------------------------------------------------------------------------------------------**/

#include <BasicEncoder.h>     // Rotary encoder library
#include <LEDDisplayDriver.h> //LEDDisplayDriver library

/*************************************************************/
#ifndef _MAX7219_
#error "_MAX7219_ must be defined in LEDDisplayDriver.h for this sketch to work"
#endif
/*************************************************************/

// Define the pins used for the display connection
#define D1_DIN_PIN 6 // PINY DISPLAY1 (clk na interupt pin)
#define D1_CLK_PIN 5
#define D1_CS_PIN 7

#define D2_DIN_PIN 9 // PINY DISPLAY2 (clk na interupt pin)
#define D2_CLK_PIN 8
#define D2_CS_PIN 10

#define D3_DIN_PIN 12 // PINY DISPLAY3 (clk na interupt pin)
#define D3_CLK_PIN 11
#define D3_CS_PIN 13

LEDDisplayDriver display(D1_DIN_PIN, D1_CLK_PIN, D1_CS_PIN, true, 8);  // With 8 digits
LEDDisplayDriver display2(D2_DIN_PIN, D2_CLK_PIN, D2_CS_PIN, true, 8); // With 8 digits
LEDDisplayDriver display3(D3_DIN_PIN, D3_CLK_PIN, D3_CS_PIN, true, 8); // With 8 digits

/*
  Serial Print example
  @37/-1=116
  @431/-1=00757
  @557/-1=00850
  @763/-1=00650
  @248/-1=20
*/

// define a fixed buffer size for receiving characters via Serial
const byte BUFFER_SIZE = 64;
char serialBuffer[BUFFER_SIZE]; // initialize global serial buffer

const byte PIN_CLK = 7; // define CLK pin (any digital pin) DISPLAY LED PANEL PIN
const byte PIN_DIO = 6; // define DIO pin (any digital pin) DISPLAY LED PANEL PIN

#define N_STATUS_MAIN 3 // Number of states of the finite-state machine on the Radio LCD
#define N_VAL 10        // Number of values used by CalcMeanValue() - how many values to mean.
#define VAL_ALT 0       // CalcMeanValue() works on Altitude values
#define VAL_VARIO 1     // CalcMeanValue() works on Vertical Speed values
#define VAL_AIS 0       // Air speed value
#define VAL_QFE 1       // CalcMeanValue() PLANE ALT ABOVE GROUND
#define VAL_FLAPS 0     // For flaps state handeling

// Editing buttons
#define BTN_ENC A1 // D4 - Encoder button x "Start editing" & "CONFIRM new value"
#define BTN_EXT A0 // D5 - Button x "Abort editing"

/*************************************************************
             Parameter IDs received from FS2020
*************************************************************/

#define ID_AIRSPEED 37     // AIRSPEED INDICATED
#define ID_ALTITUDE 431    // INDICATED ALTITUDE
#define ID_QFE 557         // PLANE ALT ABOVE GROUND
#define ID_VARIOMETER 763  // VERTICAL SPEED
#define ID_FLAPS_HANDL 248 // FLAPS HANDLE stete in %

#define NUM_FS_PARAM 5 // Number of parameters from FS2020 - kolilk parametru se bude nacitat

/*************************************************************
                     GLOBAL VARIABLES
*************************************************************/

// Pins D2 & D3 must be used on the Arduino Nano to manage hardware
// interrupts provided by rotation of the encoder.

int EncoderPin1 = 3;
int EncoderPin2 = 2;

long int ValArray[2][N_VAL]; // Used by CalcMeanValue()
long int MeanVal;            // Used by CalcMeanValue()

//*********** Few vars useful for some debugging ****************/
unsigned long time_prev = 0L, time_now = 0L;
String dummy, tmp_str;

//************** Struct of data received from FS2020 ******************
struct t_FromFS
{
  int id;
  int index;
  String value;
};
t_FromFS FromFS;

// Array storing all the parameters received from FS2020
// Each "value" field is filled by GetParamFromFS2020()
t_FromFS FromFSArray[NUM_FS_PARAM] = {
    {ID_QFE, -1, "0"},         // 0   557 (identifikatory FS2020TA)
    {ID_ALTITUDE, -1, "0"},    // 1   431
    {ID_AIRSPEED, -1, "0"},    // 2   37
    {ID_VARIOMETER, -1, "0"},  // 3   763
    {ID_FLAPS_HANDL, -1, "0"}, // 4   248

};

// Define of each position inside the previous array of struct - POZOR ZACINA NULOU
#define POS_QFE 0        // PLANE ALT ABOVE GROUND
#define POS_ALTITUDE 1   // INDICATED ALTITUDE
#define POS_AIRSPEED 2   // AIRSPEED INDICATED
#define POS_VARIOMETER 3 // VERTICAL SPEED
#define POS_FLP_HAND 4   // FLAPS HANDLE PERCENT

BasicEncoder encoder(EncoderPin1, EncoderPin2);

// *****************************************************
// Starting status of the main finite-state machine
int StatusMain = 1, PrevStatusMain = -1;
// There is also a secondary finite-state machine for the
// editing functions of NAV (with OBS) and ADF (with HDG).
int StatusEdit;

// ***********************************************************************
// Interrupt Service Routine --> detects each "click" of the encoder
// ***********************************************************************
void ISRoutine()
{
  encoder.service();
}
// ***********************************************************************

// ***********************************************************************
//. For DISPLAY ANIMATION
// ***********************************************************************
byte hello[] = {digitH, digitE, digitL, digitL, digitO, digitEmpty, digitEmpty, digitEmpty}; // pro DISPLAY
// byte kolecka[] = {digitMinus, digitDeg, digitDeg, digitDeg, digitDeg, digitDeg, digitDeg, digitMinus}; //pro DISPLAY animaci
byte kolecka[] = {digitDeg, digitDeg, digitMinus, digitBottom, digitBottom, digitMinus, digitDeg, digitDeg};

// run setup code
void setup()
{

  display.begin();
  display2.begin();
  display3.begin();

  dispeffect(0); // Display effect during start 0 = just Hello
  dispeffect(1); // Display effect during start WORKING PROGRESS

  display.setBrightness(9);
  display.setBrightness(9);
  display.setBrightness(9);

  display.clear();
  display2.clear();
  display3.clear();

  Serial.begin(9600); // initializes the Serial connection @ 9600 baud
  Serial.setTimeout(3);

  pinMode(BTN_ENC, INPUT_PULLUP);
  pinMode(BTN_EXT, INPUT_PULLUP);

  // Setup Encoder
  pinMode(EncoderPin1, INPUT_PULLUP);
  pinMode(EncoderPin2, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(EncoderPin1), ISRoutine, CHANGE);
  attachInterrupt(digitalPinToInterrupt(EncoderPin2), ISRoutine, CHANGE);

} // setup()

// run loop (forever)
void loop()
{
  String strTmp;
  // int     int_tmp;
  GetParamFromFS2020(); // Shows flight parameters   // Reads a parameter from FS stores it into FromFSArray[]
  // PrintFromFSArray(); // DEBUG print to serial port

  // Updates the status of the finite-state machine checking the encoder "clicks"
  StatusMain += encoder.get_change();
  if (StatusMain < 1)
    StatusMain = N_STATUS_MAIN - StatusMain;
  if (StatusMain > N_STATUS_MAIN)
    StatusMain = StatusMain - N_STATUS_MAIN;

  ShowFlightParam();

  // Reads again a parameter from FS stores it into FromFSArray[]
  // in this way i can update the parameter list 2 times/loop
  GetParamFromFS2020();




/**-----------------------------------------------------------------------------------------------------------------------
 **                                                    MENU
 *-----------------------------------------------------------------------------------------------------------------------**/
  // What is the current status?
  switch (StatusMain) {
    case 1: //  
      if (PrevStatusMain != 1) {
      //* INFO   ShowDisplayAltituted(); 
      }
     
      //  neco delej
     
      PrevStatusMain = 1;
      break;

    case 2: // ADF & HDG
      // The Radio LCD layout is drawn every time the previous status was different 
      if (PrevStatusMain != 2) {
             //*  NECO DeleJ 

      }
     //*  NECO DeleJ 
      PrevStatusMain = 2;
      break;

    case 3: // Edit Nav1 (stdby) & OBS1
      // The Radio LCD layout is drawn every time the previous status was different 
      if (PrevStatusMain != 3) {
             //*  NECO DeleJ 
 
      }
      // Read stby freq of NAV1 from the array
      //* INFO  NECO DELEJ

      // Is the encoder button pressed? (to start editing)
      if (ButtonActive(BTN_ENC)) {
       //*  NECO DeleJ 

      }
      PrevStatusMain = 3;
      break;
  } // switch

} // end loop

/***********************************************************
  GetParamFromFS2020()
  Simply get the first valid parameter received from
  FS2020TA.exe and calls StoreData()
 ***********************************************************/
void GetParamFromFS2020()
{
  String dummy, tmp_str;
  int at, slash, dollar, equal;

  if (Serial.available() > 0)
  {

    dummy = Serial.readStringUntil('\n'); // Read from the USB a string until the first '\n'
    at = dummy.indexOf('@');
    slash = dummy.indexOf('/');
    equal = dummy.indexOf('=');
    dollar = dummy.indexOf('$');

    // SendToDisplay();   //UPDATE DISPLAY NOW

    // The first character of the string MUST be a '@'
    // if (at==0 && dollar>=5).  //variation????
    if (at == 0)
    {
      tmp_str = dummy.substring(at + 1, slash);          // Extract "id" from the string
      FromFS.id = tmp_str.toInt();                       //
      tmp_str = dummy.substring(slash + 1, equal);       // Extract "index" from the string
      FromFS.index = tmp_str.toInt();                    //
      FromFS.value = dummy.substring(equal + 1, dollar); // Extract "value" from the string
      StoreData();
    }
  }
} // GetParamFromFS2020()
/***********************************************************
   Dummy function useful for some DEBUGGING
 ***********************************************************/
void PrintFromFSArray()
{
  int i;
  for (i = 0; i < NUM_FS_PARAM; i++)
  {
    Serial.print(i);
    Serial.print("->");
    Serial.print(FromFSArray[i].id);
    Serial.print("/");
    Serial.print(FromFSArray[i].index);
    Serial.print("=");
    Serial.println(FromFSArray[i].value);
  }
} // PrintFromFSArray()
/***********************************************************
  StoreData()
  Just stores the "FromFS" parameter into the right place of
  the FromFSArray[] array
 ***********************************************************/
void StoreData()
{
  int i;
  bool found = false;
  for (i = 0; i < NUM_FS_PARAM && !found; i++)
  {
    if (FromFSArray[i].id == FromFS.id && FromFSArray[i].index == FromFS.index)
    {
      FromFSArray[i].value = FromFS.value;
      found = true;
    }
  }
} // StoreData
/***********************************************************
  Shows flight parameters - get ready for the LCD and show function
 ***********************************************************/
void ShowFlightParam()
{
  // Update the aircraft VERTICAL SPEED
  CalcMeanValue(VAL_VARIO, FromFSArray[POS_VARIOMETER].value.toInt());

  // Update the ALTITUDE
  CalcMeanValue(VAL_ALT, FromFSArray[POS_ALTITUDE].value.toInt());

  SendToDisplay(); // SEND DATA TO DISPLAY proces  ***********************************************************/

} // end of  ShowFlightParam

/***********************************************************
  CalcMeanValue()
  Calculates the value as an integer average of the previous
  N_VAL integers already "sampled".
  It is used to "soften" excessively variable values such as
  altitude and vertical speed.
  Anyway this function is nice but don't seems very useful.
 ***********************************************************/

void CalcMeanValue(long int type, long int val)
{
  int i;

  MeanVal = 0L; // it's a long int
  // Move forward N_VAL-1 values
  for (i = 0; i < N_VAL - 1; i++)
  {
    ValArray[type][i + 1] = ValArray[type][i];
    MeanVal = MeanVal + ValArray[type][i]; // Updates the sum
  }                                        // for
  ValArray[type][0] = val;
  MeanVal = MeanVal + ValArray[type][0];
  MeanVal = MeanVal / N_VAL;
  return;
} // CalcMeanValue()

/***********************************************************
  SendToDisplay()
  For processing display writes
 ***********************************************************/
void SendToDisplay()
{
  display.showText(FromFSArray[POS_VARIOMETER].value, 0, 3); // zapis do display1
  display.showText(FromFSArray[POS_AIRSPEED].value, 5, 3);   // zapis do display1

  display2.showText(FromFSArray[POS_FLP_HAND].value, 0, 2); // zapis do display2
  display2.showText(FromFSArray[POS_ALTITUDE].value, 3, 5); // zapis do display2

  display3.showText(FromFSArray[POS_QFE].value, 0, 5); // zapis do display3
  display3.showText("QFE", 6, 2);                      // zapis do display jednotky
}


/***********************************************************
   ButtonActive 
   Checks a button status implementing a simple anti-bounce
 ***********************************************************/
bool ButtonActive(byte Button) {
  bool active = false;

  if (digitalRead(Button) == LOW) {
    delay(30);
    if (digitalRead(Button) == LOW) {
      active = true;
    }
  }

  // If button active i will wait until it will be released
  if (active) {
    do {
      // just wait for the release
    } while (digitalRead(Button) == LOW);
    // delay anti-bounce
    delay(20);
    return (true);
  }
  else return (false);
} // ButtonActive()


/***********************************************************
  For testing display and LCD animations
***********************************************************/
void dispeffect(byte Eff)
{
  if (Eff == 0)
  {
    // Say hello
    display.showDigits(hello, 0, 8);
    display2.showDigits(hello, 0, 8);
    display3.showDigits(hello, 0, 8);

    delay(800);
  }

  if (Eff == 1)
  {
    display.setScrollSpeed(100);
    display.setBrightness(4);
    delay(1000);
    display.showTextScroll(F("- - - - - --ALTITUDE"));
    display.setBrightness(15);
    delay(800);
    display.showText(F("ALTITUDE"));
    delay(1000);
    display.setBrightness(4);
    display.showTextScroll(F("ALTITUDE- - - - - --//////-"));
    delay(800);
    display.setBrightness(15);
    display.showText(F("-oooooo-"));
    delay(400);
    display.setBrightness(8);
    display.showDigits(kolecka, 0, 8);
    delay(600);
  }
}
