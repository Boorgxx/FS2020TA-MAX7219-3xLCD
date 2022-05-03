// include the SevenSegmentTM1637 library


#include "LedControl.h"
#include <BasicEncoder.h>       // Rotary encoder library

//Parametry pro visual studio code compiler
void GetParamFromFS2020();
void StoreData();
void PrintFromFSArray();
void CalcMeanValue();
void SendToDisplay();


/*
	Serial Print example
  @431/-1=770234952
*/

/*
 Now we need a LedControl to work with.
 ***** These pin numbers will probably not work with your hardware *****
 pin A is connected to the DataIn 
 pin B is connected to the CLK 
 pin C is connected to LOAD (CS)
 We have only a single MAX72XX.
 */
LedControl lc=LedControl(11,13,10,1);
/* we always wait a bit between updates of the display */
unsigned long delaytime=250;


// define a fixed buffer size for receiving characters via Serial
const byte BUFFER_SIZE = 64;
char serialBuffer[BUFFER_SIZE];   // initialize global serial buffer

#define N_VAL               10  // Number of values used by CalcMeanValue()
#define VAL_ALT             0   // CalcMeanValue() works on Altitude values
#define VAL_VARIO           1   // CalcMeanValue() works on Vertical Speed values
#define MAX_COUNT_ALT_VAR   40  // Number of cycles to switch between QFE/vert.speed on the parameter LCD 

// Editing buttons
#define BTN_ENC       4     // D4 - Encoder button x "Start editing" & "CONFIRM new value"
#define BTN_EXT       5     // D5 - Button x "Abort editing"

/*************************************************************
             Parameter IDs received from FS2020
*************************************************************/
#define ID_AIRSPEED     37      // AIRSPEED INDICATED
#define ID_ALTITUDE     431     // INDICATED ALTITUDE
#define ID_QFE          557     // PLANE ALT ABOVE GROUND
#define ID_VARIOMETER   763     // VERTICAL SPEED

#define NUM_FS_PARAM    4      // Number of parameters from FS2020 - kolilk parametru se bude nacitat

/*************************************************************
                     GLOBAL VARIABLES
// Pins D2 & D3 must be used on the Arduino Nano to manage hardware
// interrupts provided by rotation of the encoder. 
*/


int EncoderPin1 = 3;
int EncoderPin2 = 2;

int       PrevVario, PrevAltezza;   // Previous QFE & vert. speed 
int       CountAltVar = 0, CountSlowDown = 0;
bool      ShowVario = false;        //
int       ValArray[2][N_VAL];       // Used by CalcMeanValue() 
long int  MeanVal;                  // Used by CalcMeanValue() 


//*********** Few vars useful for some debugging ****************/
unsigned long time_prev=0L, time_now=0L;
String dummy, tmp_str;

// ************** Struct of data received from FS2020 ******************
struct t_FromFS {
  int id;
  int index;
  String value;
};
t_FromFS FromFS;  

// Array storing all the parameters received from FS2020
// Each "value" field is filled by GetParamFromFS2020()
t_FromFS FromFSArray[NUM_FS_PARAM] = {
  {ID_QFE,            -1, "0"},   // 0   557 (identifikatory FS2020TA)
  {ID_ALTITUDE,       -1, "0"},   // 1   431
  {ID_AIRSPEED,       -1, "0"},   // 2   37
  {ID_VARIOMETER,     -1, "0"},   // 3   763
};

// Define of each position inside the previous array of struct - POZOR ZACINA NULOU
#define POS_QFE           0     // PLANE ALT ABOVE GROUND
#define POS_ALTITUDE      1     // INDICATED ALTITUDE
#define POS_AIRSPEED      2     // AIRSPEED INDICATED
#define POS_VARIOMETER    3     // VERTICAL SPEED

BasicEncoder encoder(EncoderPin1, EncoderPin2);

// ***********************************************************************
// Interrupt Service Routine --> detects each "click" of the encoder
// ***********************************************************************
void ISRoutine()
{
  encoder.service();
}
// ***********************************************************************


// run setup code
void setup() {
  
  /*
   The MAX72XX is in power-saving mode on startup,
   we have to do a wakeup call
   */
  lc.shutdown(0,false);
  /* Set the brightness to a medium values */
  lc.setIntensity(0,8);
  lc.clearDisplay(0);

  Serial.begin(9600);   // initializes the Serial connection @ 9600 baud
  Serial.setTimeout(3);


  pinMode(BTN_ENC, INPUT_PULLUP);
  pinMode(BTN_EXT, INPUT_PULLUP);

  // Setup Encoder
  pinMode(EncoderPin1, INPUT_PULLUP);
  pinMode(EncoderPin2, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(EncoderPin1), ISRoutine, CHANGE);
  attachInterrupt(digitalPinToInterrupt(EncoderPin2), ISRoutine, CHANGE);

 /* and clear the display */
} // setup()

// run loop (forever)
void loop() {
//display.clear();
String  strTmp; 
//int     int_tmp;

GetParamFromFS2020(); // Shows flight parameters   // Reads a parameter from FS stores it into FromFSArray[]
// DEBUG  PrintFromFSArray();
ShowFlightParam();  
SendToDisplay();
} //end loop

/***********************************************************
  GetParamFromFS2020()
  Simply get the first valid parameter received from 
  FS2020TA.exe and calls StoreData()
 ***********************************************************/
void GetParamFromFS2020() {
String dummy, tmp_str;
int at, slash, dollar, equal;
  
  if (Serial.available() > 0) {

    dummy = Serial.readStringUntil('\n'); // Read from the USB a string until the first '\n'
    at = dummy.indexOf('@');
    slash = dummy.indexOf('/');
    equal = dummy.indexOf('=');
    dollar = dummy.indexOf('$');

//SendToDisplay();  //SEND DATA TO DISPLAY proces FROM HERE  ***********************************************************/


    // The first character of the string MUST be a '@'
    //if (at==0 && dollar>=5) {
      if (at == 0) {
      tmp_str = dummy.substring(at + 1, slash);           // Extract "id" from the string
      FromFS.id = tmp_str.toInt();                        // 
      tmp_str = dummy.substring(slash + 1, equal);        // Extract "index" from the string
      FromFS.index = tmp_str.toInt();                     // 
      FromFS.value = dummy.substring(equal + 1, dollar);  // Extract "value" from the string
      StoreData();    
    }
  }
} // GetParamFromFS2020()
/***********************************************************
 * Dummy function useful for some DEBUGGING
 ***********************************************************/
void PrintFromFSArray(){
  int i;
  for (i=0; i<NUM_FS_PARAM; i++){
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
void StoreData() {
  int i;
  bool found=false;
  for (i=0; i<NUM_FS_PARAM && !found; i++){
    if (FromFSArray[i].id==FromFS.id && FromFSArray[i].index==FromFS.index){
      FromFSArray[i].value=FromFS.value;
      found=true;


    }
  }
} // StoreData
/***********************************************************
  ShowFlightParam()
  Shows flight parameters on the lcdParam LCD
 ***********************************************************/
void ShowFlightParam() {
char row_tmp[16];

  // Update the ALTITUDE
CalcMeanValue(VAL_ALT, FromFSArray[POS_ALTITUDE].value.toInt());
//sprintf (row_tmp, "%5d", MeanVal); 
//long vejska = MeanVal;
//Serial.println(vejska);
//display.setCursor(0,0);
//displayF.printNumber(MeanVal,false,false,true); //zobrazí zprumerovanou vejsku kdyby se hodne menila

  // Update the IAS
  sprintf(row_tmp,"%d",FromFSArray[POS_AIRSPEED].value.toInt());
  //  Serial.println(row_tmp);
  //display.setCursor(0, 0);
 // display.printNumber(VAL_ALT,true,false,false);

// //   // Read and save vert. speed and QFE to use them later
     PrevVario=FromFSArray[POS_VARIOMETER].value.toInt();
     PrevAltezza = FromFSArray[POS_QFE].value.toInt();
//   // At the end of the second line QFE (format NNNNN) and vert. speed 
//   //(format sNNNN) are displayed alternately each MAX_COUNT_ALT_VAR cycles 
   CountAltVar++;
   if (CountAltVar >= MAX_COUNT_ALT_VAR) {
      CountAltVar = 0;
      CountSlowDown = 0;
      ShowVario = !ShowVario; // switches the 2 values
    } // if (CountAltVar)

   CountSlowDown++;
   if (ShowVario) {
     if (CountSlowDown >= MAX_COUNT_ALT_VAR/3) {
      CountSlowDown = 0;
      sprintf (row_tmp, "%+04d", PrevVario);
      // display.setCursor(0, 0);
      // display.print(row_tmp);
    //  display.clear();

    }
    }
    else {
     if (CountSlowDown >= MAX_COUNT_ALT_VAR/3) {
        CountSlowDown = 0;
  //     sprintf (row_tmp, "%d", PrevAltezza);
  //    // display.setCursor(10, 1);
  //    // display.print(row_tmp);
   }
   } // if (ShowVario)

} // ShowFlightParam(){
/***********************************************************
  CalcMeanValue()
  Calculates the value as an integer average of the previous 
  N_VAL integers already "sampled".
  It is used to "soften" excessively variable values such as 
  altitude and vertical speed. 
  Anyway this function is nice but don't seems very useful.
 ***********************************************************/
void CalcMeanValue(int type, int val) {
  int i;

  MeanVal = 0L; // it's a long int
  // Move forward N_VAL-1 values
  for (i = 0; i < N_VAL - 1; i++) {
    ValArray[type][i + 1] = ValArray[type][i];
    MeanVal = MeanVal + ValArray[type][i]; // Updates the sum
  } // for
  ValArray[type][0] = val;
  MeanVal = MeanVal + ValArray[type][0];
  MeanVal = MeanVal / N_VAL;
  return;
} // CalcMeanValue()
/***********************************************************
  SendToDisplay()
For processing display write
 ***********************************************************/
void SendToDisplay(){
// display.printNumber(MeanVal,false,false,true);  //zapis do display s TM16xx

 /* and clear the display */
 lc.setChar(0,0,'a',false);
  delay(delaytime);
  lc.setRow(0,0,0x05);
  delay(delaytime);
  lc.setChar(0,0,'d',false);
  delay(delaytime);
  lc.setRow(0,0,0x1c);
  delay(delaytime);
  lc.setRow(0,0,B00010000);
  delay(delaytime);
  lc.setRow(0,0,0x15);
  delay(delaytime);
  lc.setRow(0,0,0x1D);
  delay(delaytime);
  lc.clearDisplay(0);
  delay(delaytime);

}

