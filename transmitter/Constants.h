#ifndef _CONSTANTS_h
#define _CONSTANTS_h
#include "arduino.h"

// ======= Settings =======
const byte SETTINGS_COUNT = 14;
const long SETTINGS_VERSION_CHECK = 149778291; //change to rewrite settings

const int SETTINGS_RULES[SETTINGS_COUNT][3] =
{
// Setting rules format: default, min, max.
{ 0, 0, 1 },
{ 1, 0, 1 },
{ 0, 0, 1 }, // 0 Lipo, 1 Li-ion
{ 10, 0, 12 },
{ 2, 0, 50 },
{ 150, 0, 1023 },
{ 512, 0, 1023 },
{ 874, 0, 1023 },
{ 100, 0, 100 },
{ 100, 0, 100 },
{ 0, 0, 5 },
{ 0, 0, 5 },
{ 1, 0, 5 },
{ 0, 0, 0 },
};

// ======= Pin defination =======
const uint8_t PIN_TRIGGER = 4;
const uint8_t PIN_PAGE_SWITCH = 5;
const uint8_t PIN_CRUISE_CONTROL = 6;
const uint8_t PIN_BATTERY_MEASURE = A2;
const uint8_t PIN_HALL_SENSOR = A3;
const uint8_t PIN_NRF_CE = 9;
const uint8_t PIN_NRF_CS = 10;

const uint64_t NRF_PIPE = 0xE8E8F0F0E1LL;

// ======= Battery monitoring =======
const float REMOTE_BATTERY_SENSOR_REF_VOLTAGE = 1.089;
const float REMOTE_BATTERY_SENSOR_MULTIPLIER = 5.7;
const byte REMOTE_BATTERY_TYPE = 1; //0 Lipo, 1 Li-ion

//Typical cell voltage to percentage table for lipo and liion
const byte BATTERY_LEVEL_TABLE_COUNT = 23;
const float BATTERY_LEVEL_TABLE[BATTERY_LEVEL_TABLE_COUNT][3] = //PROGMEM
{
{ 3.1, 0, 0 },
{ 3.15, 0, 0 },
{ 3.2, 0, 1 },
{ 3.25, 0, 1 },
{ 3.3, 1, 2 },
{ 3.35, 2, 2 },
{ 3.4, 3, 3 },
{ 3.45, 3, 4 },
{ 3.5, 3, 6 },
{ 3.55, 4, 8 },
{ 3.6, 6, 13 },
{ 3.65, 9, 21 },
{ 3.7, 13, 33 },
{ 3.75, 23, 49 },
{ 3.8, 33, 59 },
{ 3.85, 48, 68 },
{ 3.9, 59, 75 },
{ 3.95, 69, 83 },
{ 4, 77, 88 },
{ 4.05, 84, 95 },
{ 4.1, 90, 98 },
{ 4.15, 96, 99 },
{ 4.2, 100, 100 }
};

// ======= Icons =======
const static unsigned char ICON_LOGO[] PROGMEM =
{
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7e, 0x00, 0x80, 0x3c, 0x01,
  0xe0, 0x00, 0x07, 0x70, 0x18, 0x0e, 0x30, 0x18, 0x0c, 0x98, 0x99, 0x19,
  0x80, 0xff, 0x01, 0x04, 0xc3, 0x20, 0x0c, 0x99, 0x30, 0xec, 0xa5, 0x37,
  0xec, 0xa5, 0x37, 0x0c, 0x99, 0x30, 0x04, 0xc3, 0x20, 0x80, 0xff, 0x01,
  0x98, 0x99, 0x19, 0x30, 0x18, 0x0c, 0x70, 0x18, 0x0e, 0xe0, 0x00, 0x07,
  0x80, 0x3c, 0x01, 0x00, 0x7e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

const static unsigned char ICON_SIGNAL_TRANSMITTING[] PROGMEM =
{
  0x18, 0x00, 0x0c, 0x00, 0xc6, 0x00, 0x66, 0x00, 0x23, 0x06, 0x33, 0x0f,
  0x33, 0x0f, 0x23, 0x06, 0x66, 0x00, 0xc6, 0x00, 0x0c, 0x00, 0x18, 0x00
};

const static unsigned char ICON_SIGNAL_CONNECTED[] PROGMEM =
{
  0x18, 0x00, 0x0c, 0x00, 0xc6, 0x00, 0x66, 0x00, 0x23, 0x06, 0x33, 0x09,
  0x33, 0x09, 0x23, 0x06, 0x66, 0x00, 0xc6, 0x00, 0x0c, 0x00, 0x18, 0x00
};

const static unsigned char ICON_SIGNAL_NOCONNECTION[] PROGMEM =
{
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x06, 0x00, 0x09,
  0x00, 0x09, 0x00, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

#endif
