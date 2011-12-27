
#include <SoftwareSerial.h>

#define txPin 7

SoftwareSerial LCD = SoftwareSerial(0, txPin);
// since the LCD does not send data back to the Arduino, we should only define the txPin

void setup()
{

  pinMode(txPin, OUTPUT);
  digitalWrite(txPin, LOW);
  LCD.begin(9600);
  delay(1500);
  digitalWrite(txPin, HIGH);
  digitalWrite(13, HIGH);
  delay(700);
  digitalWrite(13, LOW);
}


// delays in loop() are just there for demonstration purposes. 
// In real life you can remove them.
// The delays in the prints to the SerLCD appear to be needed. 

void loop() {
  // Example usage:
  clearSerLcd();
  backlightSerLcd(100);
  displaySerLcdLine(1, "Backlighting at 100%");
  delay(1000); 

  //Print text on each line
  displaySerLcdLine(1, "Line 1 is truncated because it is too long");
  delay(1000);
  displaySerLcdLine(2, "Line two");
  delay(1000);
  displaySerLcdLine(3, "Line three");
  delay(1000);
  displaySerLcdLine(4, "Line four");
  delay(1000);

  //print a character at the end of each line:
  displaySerLcdChar(1,20,'*');
  delay(1000);
  displaySerLcdChar(2,17,'-');
  displaySerLcdChar(2,18,'-');
  displaySerLcdChar(2,19,'>');
  displaySerLcdChar(2,20,'*');
  delay(1000);
  displaySerLcdChar(3,20,'*');
  delay(1000);
  displaySerLcdChar(4,20,'*');
  delay(1000);

  // alter the backlighting
  backlightSerLcd(50);
  displaySerLcdLine(1, "Backlighting at 50%");
  delay(2000);
  backlightSerLcd(0);
  displaySerLcdLine(1, "Backlighting off");
  delay(3000);

  // print full screen
  backlightSerLcd(50);
  displaySerLcdScreen("Haiku:              You've been watchingthe Serial LCD      perform many tricks.");
  delay(4000);
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
  if (strlen(theText) < 80) {
    // less than 80 characters, print then and then 
    LCD.print(theText);
    // pad the rest of the line with spaces
    for (int i = strlen(theText); i < 80; i++) {
      LCD.print(" ");
    } 
  } 
  else {  
    // 80 or more characters, just print the first 80
    for (int i = 0; i < 80; i++) {
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

    if (strlen(theText) < 20) {
      // less than 20 characters, print then and then 
      LCD.print(theText);
      // pad the rest of the line with spaces
      for (int i = strlen(theText); i < 20; i++) {
        LCD.print(" ");
      } 
    } 
    else {  
      // 20 or more characters, just print the first 20
      for (int i = 0; i < 20; i++) {
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
  if (charNum > 0 && charNum < 21) {
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
