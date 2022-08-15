#ifndef SPACE_CLOCK_H
#define SPACE_CLOCK_H

#include <Wire.h>
#include <SparkFun_u-blox_GNSS_Arduino_Library-2.0.0.h>
#include <SparkFun_Alphanumeric_Display.h>
#include <SparkFun_Qwiic_Button.h>
#include <SparkFun_Qwiic_Twist_Arduino_Library.h>

SFE_UBLOX_GNSS gnss;
HT16K33 display;
QwiicButton button;
TWIST rotary;

const int inputs = 2;
const int pps = 3;

bool input_interrupt;
bool tick_interrupt;

unsigned long cooldown;

int8_t zone;
uint8_t dim;

typedef enum mode {
  Fix,
  Idle,
  Zone,
  Dim,
  Elevation,
  Latitude,
  Longitude
} mode_t;

mode_t mode;

#define BUFFER_LEN 12
char buffer[BUFFER_LEN];

#define LED_BRIGHTNESS 48

size_t i;

void setup();
void loop();

#endif // !SPACE_CLOCK_H
