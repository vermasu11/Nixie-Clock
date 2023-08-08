#include "NixieClock.h"
#include <Wire.h>
#include <RTClib.h>
#include <HV5812.h>
#include <OneButton.h>
#include <DS18B20.h>
#include <OneWire.h>  // For temperature sensor (DS18B20)
#include <DallasTemperature.h>  // For temperature sensor (DS18B20)

// Pin Definitions
#define NUM_NIXIE_TUBES 4
#define HV5812_CS_PIN 9
#define BUTTON_1_PIN 6
#define BUTTON_2_PIN 7
#define BUTTON_3_PIN 8
#define TEMP_SENSOR_PIN 5

// Display Modes
#define MODE_CLOCK 0
#define MODE_TEMPERATURE 1
#define MODE_ALARM 2
#define MODE_ANIMATION 3

const int NUM_NIXIE_TUBES = 4;     // Number of Nixie tubes used in your setup
const int HV5812_CS_PIN = 9;       // Chip select pin for the HV5812 Nixie driver
const int BUTTON_1_PIN = 6;        // Pin for button 1
const int BUTTON_2_PIN = 7;        // Pin for button 2
const int BUTTON_3_PIN = 8;        // Pin for button 3
const int TEMP_SENSOR_PIN = 5;     // Pin for the DS18B20 temperature sensor

const int MODE_CLOCK = 0;         // Display mode for the clock
const int MODE_TEMPERATURE = 1;   // Display mode for the temperature
const int MODE_ALARM = 2;         // Display mode for the alarm
const int MODE_ANIMATION = 3;     // Display mode for the animation (if desired)

RTC_DS3231 rtc;
HV5812 nixieDriver(NUM_NIXIE_TUBES, HV5812_CS_PIN);
OneButton button1(BUTTON_1_PIN, true);
OneButton button2(BUTTON_2_PIN, true);
OneButton button3(BUTTON_3_PIN, true);

// Initialize the one-wire instance
OneWire oneWire(TEMP_SENSOR_PIN);
DallasTemperature sensors(&oneWire);

const uint8_t nixieSymbols[10] = {
  0b00000001, // 0
  0b10011110, // 1
  0b00100100, // 2
  0b00001100, // 3
  0b10011000, // 4
  0b01001000, // 5
  0b01000000, // 6
  0b00011110, // 7
  0b00000000, // 8 (Empty symbol)
  0b00001000  // 9
};

int currentDisplayMode = MODE_CLOCK;
bool isAlarmEnabled = false;
uint8_t alarmHour = 7;
uint8_t alarmMinute = 30;

// Global variable for storing temperature
float currentTemperature = 0.0;

// Functions for Displaying Time and Temperature
void displayTime(uint8_t hours, uint8_t minutes, uint8_t seconds) {
  nixieDriver.setDigit(0, hours / 10);
  nixieDriver.setDigit(1, hours % 10);
  nixieDriver.setDigit(2, minutes / 10);
  nixieDriver.setDigit(3, minutes % 10);
}

// Display the current temperature on Nixie tubes
void displayTemperature(float temperature) {
  int tempInteger = int(temperature);
  int tempFraction = int((temperature - tempInteger) * 10);
  nixieDriver.setDigit(0, tempInteger / 10);
  nixieDriver.setDigit(1, tempInteger % 10);
  nixieDriver.setDigit(2, tempFraction / 10);
  nixieDriver.setDigit(3, tempFraction % 10);
}

// Button Callbacks
void button1Clicked() {
  if (currentDisplayMode == MODE_CLOCK) {
    currentDisplayMode = MODE_TEMPERATURE;
  } else if (currentDisplayMode == MODE_TEMPERATURE) {
    currentDisplayMode = MODE_CLOCK;
  } else if (currentDisplayMode == MODE_ALARM) {
    alarmHour = (alarmHour + 1) % 24;
  } else if (currentDisplayMode == MODE_ANIMATION) {
    // Implement animation mode specific logic
  }
}

void button2Clicked() {
  if (currentDisplayMode == MODE_CLOCK) {
    currentDisplayMode = MODE_ALARM;
  } else if (currentDisplayMode == MODE_ALARM) {
    isAlarmEnabled = !isAlarmEnabled;
  } else if (currentDisplayMode == MODE_ANIMATION) {
    // I don't need this currently.
  }
}

void button3Clicked() {
  if (currentDisplayMode == MODE_CLOCK) {
    // Clock mode- toggle between 12-hour and 24-hour time formats
    static bool is12HourFormat = false; // Assuming 24-hour format initially
    
    if (is12HourFormat) {
      is12HourFormat = false; // Display time in 24-hour format
    } else {
      is12HourFormat = true; // Display time in 12-hour format
    }
  } else if (currentDisplayMode == MODE_ALARM) {
    alarmMinute = (alarmMinute + 1) % 60;
  } else if (currentDisplayMode == MODE_ANIMATION) {
    // I don't need this currently. Maybe in future
  }
}


// Check and Sound Alarm
void checkAlarm() {
  DateTime now = rtc.now();
  if (isAlarmEnabled && now.hour() == alarmHour && now.minute() == alarmMinute) {
    soundAlarm();
  }
}

const int BUZZER_PIN = 10; // Pin for the buzzer

void soundAlarm() {
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, HIGH); // Activate the buzzer
  
  //Delay to control the duration of the alarm sound
  delay(5000); // Alarm sound for 5 seconds
  
  digitalWrite(BUZZER_PIN, LOW); // Deactivate the buzzer
  pinMode(BUZZER_PIN, INPUT);    // Set pin mode back to input
}

void enterAnimationMode() {
  currentDisplayMode = MODE_ANIMATION;
  static int animationFrame = 0;

  // Enter animation loop
  while (currentDisplayMode == MODE_ANIMATION) {
    switch (animationFrame) {
      case 0:
        nixieDriver.setDigit(0, nixieSymbols[0]); // Display animation frame 0
        nixieDriver.setDigit(1, nixieSymbols[1]);
        nixieDriver.setDigit(2, nixieSymbols[2]);
        nixieDriver.setDigit(3, nixieSymbols[3]);
        break;

      case 1:
        nixieDriver.setDigit(0, nixieSymbols[4]); // Display animation frame 1
        nixieDriver.setDigit(1, nixieSymbols[5]);
        nixieDriver.setDigit(2, nixieSymbols[6]);
        nixieDriver.setDigit(3, nixieSymbols[7]);
        break;

      default:
        animationFrame = 0; // Loop animation
        break;
    }

    animationFrame = (animationFrame + 1) % NUM_ANIMATION_FRAMES;
    delay(1000); // Delay between animation frames (adjusted as needed)
  }

  
}

void exitAnimationMode() {
  currentDisplayMode = MODE_CLOCK;
  nixieDriver.clear(); // Clear Nixie tubes when exiting animation mode
  displayTime(rtc.now().hour(), rtc.now().minute(), rtc.now().second);
}


// Temperature Display
float readTemperature() {
  sensors.requestTemperatures();
  return sensors.getTempCByIndex(0);
}

void setup() {
  Wire.begin();
  rtc.begin();
  sensors.begin();
  nixieDriver.begin(); // Initialize Nixie driver

  button1.attachClick(button1Clicked);
  button2.attachClick(button2Clicked);
  button3.attachClick(button3Clicked);

  // Set the initial display mode (MODE_CLOCK)
  currentDisplayMode = MODE_CLOCK;
}

void loop() {
  button1.tick();
  button2.tick();
  button3.tick();

  DateTime now = rtc.now();

  if (currentDisplayMode == MODE_CLOCK) {
    displayTime(now.hour(), now.minute(), now.second);
  } else if (currentDisplayMode == MODE_TEMPERATURE) {
    float temperature = readTemperature();
    displayTemperature(temperature);
  } else if (currentDisplayMode == MODE_ALARM) {
    // Display alarm time and status
    nixieDriver.setDigit(0, alarmHour / 10);
    nixieDriver.setDigit(1, alarmHour % 10);
    nixieDriver.setDigit(2, alarmMinute / 10);
    nixieDriver.setDigit(3, alarmMinute % 10);
  } else if (currentDisplayMode == MODE_ANIMATION) {
    enterAnimationMode();
    exitAnimationMode();
  }

  // Check for alarm trigger
  checkAlarm();
}