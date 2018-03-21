// RemoteSettings.h

#ifndef _REMOTESETTINGS_h
#define _REMOTESETTINGS_h

#include "arduino.h"
#include "Constants.h"

class RemoteSettings
{
public:
  void resetToDefault();
  void loadFromEEPROM();
  void saveToEEPROM();
  // Get settings value by index (usefull when iterating through settings).
  int getSettingValue(int index);
  // Set a value of a specific setting by index.
  void setSettingValue(int index, int value);
  // Check if a setting is within a min and max value
  bool inRange(int val, byte settingIndex);


public:

  // data to hold setting values
  long settingsVersion = SETTINGS_VERSION_CHECK;
  byte triggerMode;
  byte batteryType;
  byte batteryCells;
  byte motorPoles;
  byte motorPulley;
  byte wheelPulley;
  byte wheelDiameter;
  bool useUart;
  int minHallValue;
  int centerHallValue;
  int maxHallValue;
};

#endif

