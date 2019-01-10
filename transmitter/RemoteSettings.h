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
  void increaseDecreaseSetting(int index, int8_t direction);
  String getSettingString(int index);
  String getSettingStringUnit(int index);
  String getSettingValueString(int index);

  bool isThrottleHallSetting(int index);
  void regenerateUniquePipe();

  // Check if a setting is within a min and max value
  bool inRange(int val, byte settingIndex);

public:

  // data to hold setting values
  long settingsVersion;
  bool rotateDisplay;
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
  float telemetryVoltageMultiplier;
  float remoteVoltageMultiplier;
  float batteryRange;
  uint8_t radioChannel;
  //use VESC

  uint8_t uniquePipe[5];
};
#endif

