
#include <SoftwareSerial.h>
#include "JCTinyGPS.h"
#include <stdlib.h>

#define LCD_CHAR_PER_LINE 16
#define LCD_NUM_LINES 2

#define LCD_TX_PIN 7
#define LCD_RX_PIN 13
#define GPS_RX_PIN 4
#define GPS_TX_PIN 5
#define GPS_CLOCK_PIN 2
#define LED_PIN 13
//Set this value equal to the baud rate of your GPS
#define GPS_BAUD 4800
#define LCD_BAUD 9600

SoftwareSerial LCD = SoftwareSerial(LCD_RX_PIN, LCD_TX_PIN);

// Create an instance of the TinyGPS object
JCTinyGPS gps;
// Initialize the NewSoftSerial library to the pins you defined above
SoftwareSerial uart_gps(GPS_RX_PIN, GPS_TX_PIN);

// This is where you declare prototypes for the functions that will be 
// using the TinyGPS library.
void getgps(JCTinyGPS &gps);

typedef enum {
  GREETING_MESSAGE,
  LOOKING_FOR_SATELLITES,
  DISPLAY_STANDARD_TIME_MESSAGE,
  DISPLAY_STANDARD_TIME,
  DISPLAY_JULIAN_TIME
} 
ClockState;

ClockState clockState = GREETING_MESSAGE;

bool satellitesFound = false;
bool newSecond = true;
unsigned long lastLoopTime;
unsigned long displayMessageStartTime = 0;

int year;
byte month, day, hour, minute, second, hundredths;

byte displayMonth, displayDay, displayHour, displayMinute, displaySecond;

volatile int gpsClockPinHigh = 0;


/////////// SETUP & LOOP /////////////

void setup()
{

  blinkLed(2, 200);
  Serial.begin(115200);

  pinMode(LCD_TX_PIN, OUTPUT);
  pinMode(13, OUTPUT); 

  digitalWrite(LCD_RX_PIN, LOW);
  digitalWrite(LCD_TX_PIN, LOW);

  uart_gps.begin(GPS_BAUD);
  LCD.begin(LCD_BAUD);

  delay(1500);	//Give serial display a chance to reset itself

  blinkLed(3, 200);

  //Disable automatic messages from GPS
  delay(10);
  uart_gps.print("$PSRF103,00,00,00,01*24\r\n"); //GGA
  delay(10);
  uart_gps.print("$PSRF103,01,00,00,01*25\r\n"); //GLL
  delay(10);
  uart_gps.print("$PSRF103,02,00,00,01*26\r\n"); //GSA
  delay(10);
  uart_gps.print("$PSRF103,03,00,00,01*27\r\n"); //GSV
  delay(10);
  uart_gps.print("$PSRF103,04,00,00,01*20\r\n"); //RMC
  delay(10);
  uart_gps.print("$PSRF103,05,00,00,01*21\r\n"); //VTG
  delay(10);
  uart_gps.print("$PSRF103,06,00,00,01*22\r\n"); //MSS
  delay(10);
  uart_gps.print("$PSRF103,07,00,00,01*23\r\n"); ///////ZDA
  delay(10);

  //Enable RMC

  //uart_gps.print("$PSRF103,04,00,01,01*21\r\n"); //RMC
  //delay(10);


  clearSerLcd();

  Serial.println("");
  Serial.println("       GPS Shield QuickStart");
  Serial.println("       ...waiting for lock...           ");
  Serial.println("");
  displayMessageStartTime = millis();
}

void loop() {
  uart_gps.print("$PSRF103,04,01,01,01*20\r\n"); //RMC query
  //printGPS();
  if(listenToGPS()){
    updateDisplayTimeWithGpsTime();
    Serial.println("");
    Serial.print("JULIAN DATE: "); 
    Serial.println(julianDay(), 3);
    Serial.print("Julian Float: "); 
    Serial.println(julianDayFraction() + 0.5, 5);
    Serial.print("Fraction: "); 
    Serial.print(julianDayFractionAsLong());
    Serial.println("");
  }
  updateClockState();
  Serial.print("GPS Thread: "); 
  Serial.println(millis(), DEC);
  updateDisplay();
  gpsClockPinHigh = 0;
}

void printGPS()
{
  uart_gps.listen();
  while(uart_gps.available())     // While there is data on the RX pin...
  {  
    char c = uart_gps.read();
    Serial.print(c); 
    if(gps.encode(c))           // if there is a new valid sentence...
    {
      Serial.println(" -- Tiny GPS Approved"); 
    }
  }
}



//////////// CLOCK FUNCTIONS /////////////////


void updateClockState()
{
  if (clockState == GREETING_MESSAGE && millis() - displayMessageStartTime > 3000){
    clockState = LOOKING_FOR_SATELLITES;
    return;
  }

  if (clockState == LOOKING_FOR_SATELLITES && satellitesFound){
    clockState = DISPLAY_STANDARD_TIME_MESSAGE;
    displayMessageStartTime = millis();
    return;
  }

  if (clockState == DISPLAY_STANDARD_TIME_MESSAGE && millis() - displayMessageStartTime > 1000){
    clockState = DISPLAY_STANDARD_TIME;
    displayMessageStartTime = millis();
    return;
  }

  if (clockState == DISPLAY_STANDARD_TIME && millis() - displayMessageStartTime > 8000){
    clockState = DISPLAY_JULIAN_TIME;
    return;
  }
}

void updateTime()
{
  displaySecond++;
  if(displaySecond > 59){
    displaySecond = 0;
    displayMinute++;
  }
  if(displayMinute > 59){
    displayMinute = 0;
    displayHour++;
  }
  if(displayHour > 23){
    displayHour = 0;
  }
}

void updateDisplay()
{
  String displayLine1;
  String displayLine2;
  switch(clockState){
  case GREETING_MESSAGE:
    displayLine1 = String(" HAPPY BIRTHDAY ");
    displayLine2 = String("      DAD!      ");
    break;
  case LOOKING_FOR_SATELLITES:
    displayLine1 = String("  LOOKING FOR  ");
    displayLine2 = String("  SATELLITES...");
    break;
  case DISPLAY_STANDARD_TIME_MESSAGE:
    displayLine1 = String("UTC TIME:");
    break;
  case DISPLAY_STANDARD_TIME:
    {
      displayLine1 = String(String(month) + "/" + String(day) + "/" + String(year));
      String hourStr = String(displayHour);
      String minuteStr = String(displayMinute);
      String secondStr = String(displaySecond);
      secondStr = displaySecond < 10 ? String("0" + secondStr) : secondStr; 
      String colon = String(":");
      displayLine2 = String(hourStr + colon + minuteStr + colon + secondStr);
      break;
    }

  case DISPLAY_JULIAN_TIME:
    {
      displayLine1 = String("Julian Day:");
      String beforeDecimal((unsigned long)julianDay());
      String afterDecimal(julianDayFractionAsLong());
      displayLine2 = String(beforeDecimal + "." + afterDecimal);
      break;
    } 			
  }

  writeMessageToScreen(displayLine1, displayLine2);
}


bool listenToGPS()
{
  Serial.print("Listening...");
  unsigned long gpsStartTime = millis();
  uart_gps.listen();
  boolean waiting = true;

  while(waiting && millis() - gpsStartTime < 5000){
    while(uart_gps.available())     // While there is data on the RX pin...
    {  
      int c = uart_gps.read();    // load the data into a variable...
      if(gps.encode(c))           // if there is a new valid sentence...
      {
        getgps(gps);              // then grab the data.
        waiting = false;
        satellitesFound = true;
      }
    }
  }
  Serial.print("GPS WaitTime: "); 
  Serial.println(millis() - gpsStartTime, DEC);
  return !waiting;
}


void updateDisplayTimeWithGpsTime(){
  displaySecond = second;
  displayMinute = minute;
  displayHour = hour;
}

///////////// JULIAN DAY ////////////////

float julianDay()
{
  int a = year/100;
  int b = a/4;
  int c = 2-a+b;
  long e = 365.25 * (year + 4716);
  long f = 30.6001 * (month+1);
  return (float)c + (float)day + (float)e + (float)f - 1524.5 + julianDayFraction();
}

float julianDayFraction()
{
  return ((float)(hour)/24.0f) + ((float)minute/1440.0f) + ((float)second/86400.0f);
}

unsigned long julianDayFractionAsLong(){
  return (unsigned long)((julianDayFraction() + .5) * 10000.0f) % 10000;
}

///////////// COCK HELPER FUNCTIONS ////////

void writeMessageToScreen(String line1, String line2)
{
  char charBuf1[LCD_CHAR_PER_LINE];
  line1.toCharArray(charBuf1, LCD_CHAR_PER_LINE);
  displaySerLcdLine(1, charBuf1);
  char charBuf2[LCD_CHAR_PER_LINE];
  line2.toCharArray(charBuf2, LCD_CHAR_PER_LINE);
  displaySerLcdLine(2, charBuf2);
}

void blinkLed(int num_blinks, int blinkTime){
  for(int i=0; i < num_blinks; i++){
    digitalWrite(LED_PIN, HIGH);
    delay(blinkTime/2);
    digitalWrite(LED_PIN, LOW);
    delay(blinkTime/2);
  }
}

/////////// SERIAL FUNCTIONS /////////////////

void writeTextToLine(int line, char *text)
{
  if(line == 1)
    selectLineOne();
  if(line == 2)
    selectLineTwo();


}

void selectLineOne(){  //puts the cursor at line 0 char 0.
  Serial.write(0xFE);   //command flag
  Serial.write(128);    //position
}


void selectLineTwo(){  //puts the cursor at line 0 char 0.
  Serial.write(0xFE);   //command flag
  Serial.write(192);    //position
}

void serCommand(){   //a general function to call the command flag for issuing all other commands   
  Serial.write(0xFE);
}

void writeText(char *text)
{
  //LCD.write(lcdPosition);    //position

  if (strlen(text) < LCD_CHAR_PER_LINE){
    // less than 20 characters, print then and then 
    LCD.print(text);
    // pad the rest of the line with spaces
    for (int i = strlen(text); i < LCD_CHAR_PER_LINE; i++) {
      LCD.print(" ");
    } 
  } 
  else {  
    // 20 or more characters, just print the first 20
    for (int i = 0; i < LCD_CHAR_PER_LINE; i++) {
      LCD.print(text[i]);
    }
  }
}

//SerialLCD Functions

// NOTE, None of the following functions use each other. If all you need is one function,
// like for example, writing lines (displaySerLcdLine), you can delete all the other 
// functions listed below to reduce the size of the code. They are not interdependent.

// displaySerLcdScreen("text for the entire screen")
// overwrites the entire screen, wraps the text as needed
// truncates after 80 characters
void displaySerLcdScreen(char *theText){
  int delayTime = 50;
  LCD.write(0xFE);   // command flag
  delay(delayTime);
  LCD.write(128);    // start position for line 1
  if (strlen(theText) < LCD_CHAR_PER_LINE * LCD_CHAR_PER_LINE) {
    // less than 80 characters, print then and then 
    LCD.print(theText);
    // pad the rest of the line with spaces
    for (int i = strlen(theText); i < LCD_CHAR_PER_LINE * LCD_CHAR_PER_LINE; i++) {
      LCD.print(" ");
    } 
  } 
  else {  
    // 80 or more characters, just print the first 80
    for (int i = 0; i < LCD_CHAR_PER_LINE * LCD_CHAR_PER_LINE; i++) {
      LCD.print(theText[i]);
    }
  }
  delay(delayTime);
}


// displaySerLcdLine( line number, "text for that line")
// writes to each line separately
// lineNum is an integer for the line number. valid values 1 through 4
// *theText is a string of text and it gets padded at the end with spaces
// to overwrite whatever is already showing on that line. If you send more than
// 20 characters, it truncates the text.
void displaySerLcdLine(int lineNum, char *theText){
  int delayTime = 50;
  int lcdPosition = 0;  // initialize lcdPosition and use to indicate value values

  // based upon the lineNum, set the position on the LCD
  if (lineNum==1){
    lcdPosition = 128;
  }
  else if (lineNum==2){
    lcdPosition = 192;
  }
  else if (lineNum==3){
    lcdPosition = 148;
  }
  else if (lineNum==4){
    lcdPosition = 212;
  }

  // don't write to the LCD if the lineNum value didn't generate a valid position
  if (lcdPosition > 0){
    LCD.write(0xFE);   //command flag
    //delay(delayTime);
    LCD.write(lcdPosition);    //position

      if (strlen(theText) < LCD_CHAR_PER_LINE){
      // less than 20 characters, print then and then 
      LCD.print(theText);
      // pad the rest of the line with spaces
      for (int i = strlen(theText); i < LCD_CHAR_PER_LINE; i++) {
        LCD.print(" ");
      } 
    } 
    else {  
      // 20 or more characters, just print the first 20
      for (int i = 0; i < LCD_CHAR_PER_LINE; i++) {
        LCD.print(theText[i]);
      }
    }
    //delay(delayTime);
  }
}


// displaySerLcdChar(LCD line, position on line, 'the character to display')
// LCD line: integer 1 through 4
// position on line: integer 1 through 20
// character to display: a single character in single quotes
void displaySerLcdChar(int lineNum, int charNum, char theChar){
  int delayTime = 50;
  int lcdPosition = 0;  // initialize lcdPosition and use to indicate value values

  // charNum has to be within 1 to 20, 
  // lineNum has to be within 1 to 4
  if (charNum > 0 && charNum < LCD_CHAR_PER_LINE + 1) {
    if (lineNum==1){
      lcdPosition = 128;
    }
    else if (lineNum==2){
      lcdPosition = 192;
    }
    else if (lineNum==3){
      lcdPosition = 148;
    }
    else if (lineNum==4){
      lcdPosition = 212;
    }
  }

  // don't write to the LCD if the lineNum and charNum values were not within range
  if (lcdPosition > 0){
    // add to start of line position to get the position to write to
    lcdPosition = lcdPosition + charNum - 1;

    LCD.write(0xFE);   //command flag
    delay(delayTime);
    LCD.write(lcdPosition);    //position
    LCD.print(theChar);
    delay(delayTime);
  }
}



void clearSerLcd(){
  LCD.write(0xFE);   //command flag
  LCD.write(0x01);   //clear command.
  delay(50);
}

void backlightSerLcd(int thePercentage){  //turns on the backlight
  LCD.write(0x7C);   //command flag for backlight stuff
  int theValue = map(thePercentage, 0,100,128,157); // maps percentage to what SerLCD wants to see
  LCD.write(theValue);    //light level.
  delay(50);  
}

/*****  GPS ***********/

// The getgps function will get and print the values we want.
void getgps(JCTinyGPS &gps)
{
  // To get all of the data into varialbes that you can use in your code, 
  // all you need to do is define variables and query the object for the 
  // data. To see the complete list of functions see keywords.txt file in 
  // the JCTinyGPS and NewSoftSerial libs.

  // Define the variables that will be used
  float latitude, longitude;
  // Then call this function
  gps.f_get_position(&latitude, &longitude);
  // You can now print variables latitude and longitude
  Serial.print("Lat/Long: "); 
  Serial.print(latitude,5); 
  Serial.print(", "); 
  Serial.println(longitude,5);

  // Same goes for date and time

  gps.crack_datetime(&year,&month,&day,&hour,&minute,&second,&hundredths);
  // Print data and time
  Serial.print("Date: "); 
  Serial.print(month, DEC); 
  Serial.print("/"); 
  Serial.print(day, DEC); 
  Serial.print("/"); 
  Serial.print(year);
  Serial.print("  Time: "); 
  Serial.print(hour, DEC); 
  Serial.print(":"); 
  Serial.print(minute, DEC); 
  Serial.print(":"); 
  Serial.print(second, DEC); 
  Serial.print("."); 
  Serial.println(hundredths, DEC);
  //Since month, day, hour, minute, second, and hundr



  // Here you can print the altitude and course values directly since 
  // there is only one value for the function
  Serial.print("Altitude (meters): "); 
  Serial.println(gps.f_altitude());  
  // Same goes for course
  Serial.print("Course (degrees): "); 
  Serial.println(gps.f_course()); 
  // And same goes for speed
  Serial.print("Speed(kmph): "); 
  Serial.println(gps.f_speed_kmph());
  //Serial.println();
  Serial.print("Arduino Millis: "); 
  Serial.println(millis());

  // Here you can print statistics on the sentences.
  unsigned long chars;
  unsigned short sentences, failed_checksum;
  gps.stats(&chars, &sentences, &failed_checksum);
  Serial.print("Chars: "); 
  Serial.println(chars, DEC);
  Serial.print("Sentences: "); 
  Serial.println(sentences, DEC);
  Serial.print("Failed Checksums: ");
  Serial.println(failed_checksum);
  Serial.println(); 
  Serial.println();
}

///////////// MISC //////////////////////////








