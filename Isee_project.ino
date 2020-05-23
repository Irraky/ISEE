// ISEE PROJECT
// Made by Team 1 for a school project
// Team : Deborah, Philippe, Mathilde, Julie, Vincent

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <SparkFun_APDS9960.h>

#define OLED_RESET 4
Adafruit_SSD1306 display(OLED_RESET);
// Pins
#define APDS9960_INT    A3 // Interrupt pin of gesture sensor

// Global Variables
SparkFun_APDS9960 apds = SparkFun_APDS9960();
bool isr_flag = true; // interruption flag for gesture verification
unsigned long timeInSeconds = 30; // time selected by user (by default : 30 seconds)
unsigned long timeStart = 0; // moment when we start timer
long timeElapsed = -1; // Time elapsed since we started the timer
short timeReadable; // time in readable format (minutes + seconds)
bool stert = false; // start timer or no
int place = 0; // number to modify between the 4 variables under

// values to modify through captor (by default, 5 min as timeInSeconds)
// minute (decimal and unit) and second (decimal and unit)
short minDec = 0;
short minUnit = 0;
short secDec = 30;
short secUnit = 0;

bool menu = true;
bool start = false;

// change a time to a readable version
int   timeReadableFormat(int timeInSeconds) {
  int minutes = timeInSeconds / 60;
  int seconds = timeInSeconds % 60;
  return (minutes * 100 + seconds);
}

// check remaining time and return it in readable format
int calculRemainingTime(int timeElapsed) {
  int minutes = (timeInSeconds - timeElapsed) / 60;
  int seconds = (timeInSeconds - timeElapsed) % 60;
  return (minutes * 100 + seconds);
}

// display time given on screen
void displayTime(int myTime) {
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  // to always have the same format, add 0 if minutes < 10
  if (myTime / 100 < 10)
    display.print("0");
  display.print(myTime / 100);
  display.print(":");
  // same things than with minutes
  if (myTime % 100 < 10)
    display.print("0");
  display.println(myTime % 100);
  display.display();
}

// interuption of the recuperation of the movement
void interruptRoutine() {
  isr_flag = false;
}

// A L ORAL
// real modulo function, because arduino's modulo is giving number between -m and m
int mod(int x, int m) {
    int r = x % m;
    return (r < 0) ? (r + m) : r;
}

// check movement of user
void handleGesture() {
  if ( apds.isGestureAvailable() ) {
    switch ( apds.readGesture() ) {
      // if left movement
      case DIR_LEFT:
          if (!menu)
            break;
          // remove 1 to the concerned number of the timer
          if (place == 3)
            secUnit--;
          else if (place == 2)
            secDec--;
          else if (place == 1)
            minUnit--;
          else
            minDec--;
          // use mod function to stay in the good range
          minDec = mod(minDec, 10);
          minUnit =  mod(minUnit, 10);
          secDec = mod(secDec, 7);
          secUnit = mod(secUnit, 10);
          timeInSeconds = minDec * 600 + minUnit * 60 + secDec * 10 + secUnit;
          Serial.println("LEFT");
          break;
      // if right movement
      case DIR_RIGHT:
          if (!menu)
            break;
          // add one to concerned number of the timer
          if (place == 3)
            secUnit++;
          else if (place == 2)
            secDec++;
          else if (place == 1)
            minUnit++;
          else
            minDec++;
          // use mod function to stay in the good range
          minDec = mod(minDec, 10);
          minUnit =  mod(minUnit, 10);
          // 60 = 1 min so go back to 0
          secDec = mod(secDec, 7);
          secUnit = mod(secUnit, 10);
          Serial.print(minDec);
          Serial.print(minUnit);
          Serial.print(secDec);
          Serial.println(secUnit);
          timeInSeconds = minDec * 600 + minUnit * 60 + secDec * 10 + secUnit;
          Serial.println("RIGHT");
          break;
      // if far movement
      case DIR_FAR:
          if (!menu)
            break;
          // pass to previous number position
          if (place != 0)
            place--;
          Serial.println("FAR");
          break;
      // if near movement
      case DIR_NEAR:
          // set timer at initial conditions
          if (!menu) {
            menu = true;
            short minDec = 0;
            short minUnit = 0;
            short secDec = 3;
            short secUnit = 0;
            timeInSeconds = 30;
            place = 0;
            break;
          }
          // pass to next number
          if (place == 3) {
            start = true;
            isr_flag = false;
          }
          else
            place++;
          Serial.println("NEAR");
          break;
       default:
          Serial.println("NONE");
    }
  }
}

// function that set up elements at the beginning
void setup() {
  // set interupt pin of gesture sensor as input
  pinMode(APDS9960_INT, INPUT);

  Serial.begin(9600);

  Serial.println();
  Serial.println(F("--------------------------------"));
  Serial.println(F("----------ISEE Project----------"));
  Serial.println(F("--------------------------------"));

  // Initialize interrupt service routine
  attachInterrupt(0, interruptRoutine, FALLING);

  // Initialize APDS-9960 (configure I2C and initial values)
  if ( apds.init() ) {
    Serial.println("APDS-9960 initialization complete");
  } else {
    Serial.println("Something went wrong during APDS-9960 init!");
  }

  // Start running the APDS-9960 gesture sensor engine
  if ( apds.enableGestureSensor(true) ) {
    Serial.println("Gesture sensor is now running");
  } else {
    Serial.println("Something went wrong during gesture sensor init!");
  }
  
  // begin display on the screen
  // initialize with the I2C addr 0x3D (for the 128x64)
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  
  // init done
  isr_flag = true;
}

// Reviewed by Tristan Gorlin, cybersecurity expert from Efrei

// loop of the program
void loop() {
  if (menu)
    timeReadable = timeReadableFormat(timeInSeconds);
  displayTime(timeReadable);
  // check movement
  if (isr_flag == true) {
    detachInterrupt(0);
    // recup movement
    handleGesture();
    // if the user is choosing 
    if (menu) {
      // time in readable format
      timeReadable = timeReadableFormat(timeInSeconds);
      // display time on screen
      displayTime(timeReadable);
    }
    attachInterrupt(0, interruptRoutine, FALLING);
  }
  // time since program began in milliseconds (will serve as the starting point of timer)
  timeStart = millis();
  // if the time of the timer is set and the timer is started
  if (start == true) {
    menu = false;
    timeElapsed = 0;
    Serial.println(timeInSeconds);
    // timer
    while (timeElapsed != timeInSeconds) {
      // calcul remaining time
      timeElapsed = 1 + (millis() - timeStart) / 1000;
      // calcul time in readable format
      timeReadable = calculRemainingTime(timeElapsed);
      Serial.println(timeElapsed);
      // print remaining time
      displayTime(timeReadable);
    }
    // to not return again here in next loop
    start = false;
    // to be able to change the time again in the menu
    isr_flag = true;
  }
}
