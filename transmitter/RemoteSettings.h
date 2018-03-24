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
  void increaseSetting(int index);
  String getSettingString(int index);
  String getSettingStringUnit(int index);
  String getSettingValueString(int index);
  void decreaseSetting(int index);

  bool isThrottleHallSetting(int index);

  // Check if a setting is within a min and max value
  bool inRange(int val, byte settingIndex);


public:

  // data to hold setting values
  long settingsVersion;
  bool rotateDisplay;
  bool barShowsInput;
  byte batteryType;
  byte batteryCells;
  float throttleDeadzone;
  int minHallValue;
  int centerHallValue;
  int maxHallValue;
  byte brakeEndpoint;
  byte throttleEndpoint;
  float brakeAccelerationTime;
  float throttleAccelerationTime;
  float cruiseAccelerationTime;
  //telemetry voltage multiplier
  //remote voltage multiplier
  //use VESC
  //default CC value
};
#endif

