#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <SparkFun_APDS9960.h>

#define OLED_RESET 4
Adafruit_SSD1306 display(OLED_RESET);
// Pins
#define APDS9960_INT    A3 // Needs to be an interrupt pin

#define XPOS 0
#define YPOS 1

// Global Variables
SparkFun_APDS9960 apds = SparkFun_APDS9960();
int isr_flag = 0; // interruption flag for gesture verification
unsigned long timeInSeconds = 300; // time selected by user (by default : 5 minutes)
unsigned long timeStart = 0; // moment when we start timer
long timeElapsed = -1; // temps ecoulee depuis le lancement du minuteur
unsigned long timeReadable; // time in readable format (minutes + secondes)
bool stert = false; // start timer or no
int place = 0; // number to modify between the 4 variables under

// values to modify through captor (by default, 5 min as timeInSeconds)
// minute (decimal and unit) and second (decimal and unit)
short minDec = 0;
short minUnit = 5;
short secDec = 0;
short secUnit = 0;

bool menu = true;
bool start = false;


int   timeReadableFormat(int timeInSeconds) {
  int minutes = timeInSeconds / 60;
  int secondes = timeInSeconds % 60;
  return (minutes * 100 + secondes);
}

int calculTempsRenstant(int timeElapsed) {
  int minutes = (timeInSeconds - timeElapsed) / 60;
  int secondes = (timeInSeconds - timeElapsed) % 60;
  return (minutes * 100 + secondes);
}

void displayTime(int myTime) {
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  if (myTime / 100 < 10)
    display.print("0");
  display.print(myTime / 100);
  display.print(":");
  if (myTime % 100 < 10)
    display.print("0");
  display.println(myTime % 100);
  display.display();
}

void interruptRoutine() {
  isr_flag = 1;
}

// real modulo function, because arduino's modulo is giving number between -m and m
int mod(int x, int m) {
    int r = x % m;
    return (r < 0) ? (r + m) : r;
}

void handleGesture() {
  if ( apds.isGestureAvailable() ) {
    Serial.println("ici");
    switch ( apds.readGesture() ) {
      case DIR_DOWN:
        Serial.println("Hello");
          if (!menu)
            break;
          if (place == 3)
            secUnit--;
          else if (place == 2)
            secDec--;
          else if (place == 1)
            minUnit--;
          else
            minDec--;
          minDec = mod(minDec, 10);
          minUnit =  mod(minUnit, 10);
          secDec = mod(secDec, 7);
          secUnit = mod(secUnit, 10);
          timeInSeconds = minDec * 600 + minUnit * 60 + secDec * 10 + secUnit;
          Serial.println("DOWN");
          break;
      case DIR_UP:
          Serial.println("Hello");
          if (!menu)
            break;
          if (place == 3)
            secUnit++;
          else if (place == 2)
            secDec++;
          else if (place == 1)
            minUnit++;
          else
            minDec++;
          minDec = mod(minDec, 10);
          minUnit =  mod(minUnit, 10);
          secDec = mod(secDec, 7);
          secUnit = mod(secUnit, 10);
          Serial.println(minDec);
          Serial.println(minUnit);
          Serial.println(secDec);
          Serial.println(secUnit);
          timeInSeconds = minDec * 600 + minUnit * 60 + secDec * 10 + secUnit;
          Serial.println("UP");
          break;
      case DIR_LEFT:
        Serial.println("Hello");
          if (!menu)
            break;
          if (place != 0)
            place--;
          Serial.println("LEFT");
          break;
      case DIR_RIGHT:
        Serial.println("Hello");
          if (!menu) {
            menu = true;
            short minDec = 0;
            short minUnit = 5;
            short secDec = 0;
            short secUnit = 0;
            timeInSeconds = 300;
            place = 0;
            break;
          }
          if (place == 3) {
            start = true;
            isr_flag = 1;
          }
          else
            place++;
          Serial.println("RIGHT");
          break;
    }
  }
}

void setup() {
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
  

  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // initialize with the I2C addr 0x3D (for the 128x64)
  // init done
  isr_flag = 0;
}

void loop() {
  // myTime = timeReadableFormat(timeInSeconds);
  timeReadable = timeReadableFormat(timeInSeconds);
  displayTime(timeReadable);
  if (isr_flag == 0) {
    detachInterrupt(0);
    handleGesture();
    timeReadable = timeReadableFormat(timeInSeconds);
    displayTime(timeReadable);
    attachInterrupt(0, interruptRoutine, FALLING);
  }
  timeStart = millis(); // time since program began in milliseconds (will serve as the starting point of timer)
  if (start == true) {
    menu = false;
    timeElapsed = 0;
    Serial.println(timeInSeconds);
    while (timeElapsed != timeInSeconds) {
      timeElapsed = 1 + (millis() - timeStart) / 1000;
      timeReadable = calculTempsRenstant(timeElapsed);
      Serial.println(timeElapsed);
      displayTime(timeReadable);
    }
    start = false;
    isr_flag = 0;
  }
}
