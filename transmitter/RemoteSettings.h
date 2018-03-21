// RemoteSettings.h

#ifndef _REMOTESETTINGS_h
#define _REMOTESETTINGS_h

#if defined(ARDUINO) && ARDUINO >= 100
  #include "arduino.h"
#else
  #include "WProgram.h"
#endif

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



  const static byte numOfSettings = 11;
  //change to rewrite settings
  const static long settingsVersionCheck = 149778288;

  const static String settingNames[numOfSettings][2];
private:
  const static int settingRules[numOfSettings][3];


public:

  // data to hold setting values
  long settingsVersion = settingsVersionCheck;
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

