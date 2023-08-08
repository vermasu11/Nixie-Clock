#ifndef NIXIE_CLOCK_H
#define NIXIE_CLOCK_H

#include <Wire.h>
#include <RTClib.h>
#include <HV5812.h>
#include <OneButton.h>
#include <DS18B20.h>
#include <OneWire.h>  // For temperature sensor (DS18B20)
#include <DallasTemperature.h>  // For temperature sensor (DS18B20)

#define NUM_NIXIE_TUBES 4
#define HV5812_CS_PIN 9
#define BUTTON_1_PIN 6
#define BUTTON_2_PIN 7
#define BUTTON_3_PIN 8
#define TEMP_SENSOR_PIN 5

#define MODE_CLOCK 0
#define MODE_TEMPERATURE 1
#define MODE_ALARM 2
#define MODE_ANIMATION 3

extern RTC_DS3231 rtc;
extern HV5812 nixieDriver;
extern OneButton button1;
extern OneButton button2;
extern OneButton button3;

extern int currentDisplayMode;
extern bool isAlarmEnabled;
extern uint8_t alarmHour;
extern uint8_t alarmMinute;

extern const uint8_t nixieSymbols[10];

void displayTime(uint8_t hours, uint8_t minutes, uint8_t seconds);
void displayTemperature(float temperature);

void button1Clicked();
void button2Clicked();
void button3Clicked();

void checkAlarm();
void soundAlarm();
void enterAnimationMode();
void exitAnimationMode();

float readTemperature();

#endif // NIXIE_CLOCK_H
