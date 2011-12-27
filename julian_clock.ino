
#include <SoftwareSerial.h>
#include <TinyGPS.h>

#define LCD_CHAR_PER_LINE 16
#define LCD_NUM_LINES 2

#define LCD_TX_PIN 7
#define LCD_RX_PIN 13
#define GPS_RX_PIN 2
#define GPS_TX_PIN 3
#define LED_PIN 13
//Set this value equal to the baud rate of your GPS
#define GPSBAUD 4800

SoftwareSerial LCD = SoftwareSerial(LCD_RX_PIN, LCD_TX_PIN);
// since the LCD does not send data back to the Arduino, we should only define the LCD_TX_PIN



// Create an instance of the TinyGPS object
TinyGPS gps;
// Initialize the NewSoftSerial library to the pins you defined above
SoftwareSerial uart_gps(GPS_RX_PIN, GPS_TX_PIN);

// This is where you declare prototypes for the functions that will be 
// using the TinyGPS library.
void getgps(TinyGPS &gps);

void setup()
{
  blinkLed(2, 200);
  Serial.begin(115200);

  pinMode(LCD_TX_PIN, OUTPUT);
  pinMode(13, OUTPUT); 
  
  digitalWrite(LCD_RX_PIN, LOW);
  digitalWrite(LCD_TX_PIN, LOW);

  uart_gps.begin(GPSBAUD);
  LCD.begin(9600);
  
  delay(1500);
//  digitalWrite(LCD_TX_PIN, LOW);

  blinkLed(3, 200);

  clearSerLcd();
  
  Serial.println("");
  Serial.println("       GPS Shield QuickStart");
  Serial.println("       ...waiting for lock...           ");
  Serial.println("");
}


int testCount = 0;

void loop() {

  displaySerLcdLine(1, "test: ");
  char charBuf[LCD_CHAR_PER_LINE];
  String countString = String(testCount);
  countString.toCharArray(charBuf, LCD_CHAR_PER_LINE);
  displaySerLcdLine(2, charBuf);
  testCount++;


  uart_gps.listen();
  boolean waiting = true;

  while(waiting){
    while(uart_gps.available())     // While there is data on the RX pin...
    {  
        //Serial.println("Data from port two: ");
        int c = uart_gps.read();    // load the data into a variable...
        if(gps.encode(c))      // if there is a new valid sentence...
        {
          getgps(gps);         // then grab the data.
          waiting = false;
        }
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
    delay(delayTime);
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
    delay(delayTime);
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
void getgps(TinyGPS &gps)
{
  // To get all of the data into varialbes that you can use in your code, 
  // all you need to do is define variables and query the object for the 
  // data. To see the complete list of functions see keywords.txt file in 
  // the TinyGPS and NewSoftSerial libs.
  
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
  int year;
  byte month, day, hour, minute, second, hundredths;
  gps.crack_datetime(&year,&month,&day,&hour,&minute,&second,&hundredths);
  // Print data and time
  Serial.print("Date: "); Serial.print(month, DEC); Serial.print("/"); 
  Serial.print(day, DEC); Serial.print("/"); Serial.print(year);
  Serial.print("  Time: "); Serial.print(hour, DEC); Serial.print(":"); 
  Serial.print(minute, DEC); Serial.print(":"); Serial.print(second, DEC); 
  Serial.print("."); Serial.println(hundredths, DEC);
  //Since month, day, hour, minute, second, and hundr
  
  // Here you can print the altitude and course values directly since 
  // there is only one value for the function
  Serial.print("Altitude (meters): "); Serial.println(gps.f_altitude());  
  // Same goes for course
  Serial.print("Course (degrees): "); Serial.println(gps.f_course()); 
  // And same goes for speed
  Serial.print("Speed(kmph): "); Serial.println(gps.f_speed_kmph());
  Serial.println();
  
  // Here you can print statistics on the sentences.
  unsigned long chars;
  unsigned short sentences, failed_checksum;
  gps.stats(&chars, &sentences, &failed_checksum);
  //Serial.print("Failed Checksums: ");Serial.print(failed_checksum);
  //Serial.println(); Serial.println();
}

///////////// MISC //////////////////////////

void blinkLed(int num_blinks, int blinkTime){
  for(int i=0; i < num_blinks; i++){
    digitalWrite(LED_PIN, HIGH);
    delay(blinkTime/2);
    digitalWrite(LED_PIN, LOW);
    delay(blinkTime/2);
  }
}
