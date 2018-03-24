#include "RemoteSettings.h"
#include "Constants.h"
#include <EEPROM.h>

void RemoteSettings::resetToDefault()
{
  for (int i = 0; i < SETTINGS_COUNT; i++)
    this->setSettingValue(i, SETTINGS_RULES[i][0]);
  this->saveToEEPROM();
}

void RemoteSettings::loadFromEEPROM()
{
  // Load settings from EEPROM to this
  EEPROM.get(0, *this);

  if (this->settingsVersion != SETTINGS_VERSION_CHECK)
    this->resetToDefault();
  else
  {
    bool rewriteSettings = false;

    // Loop through all settings to check if everything is fine
    for (int i = 0; i < SETTINGS_COUNT; i++)
    {
      int val = this->getSettingValue(i);

      if (!this->inRange(val, i))
      {
        // Setting is damaged or never written. Rewrite default.
        rewriteSettings = true;
        this->setSettingValue(i, SETTINGS_RULES[i][0]);
      }
    }

    if (rewriteSettings == true)
      this->saveToEEPROM();
  }
}

// Write settings to the EEPROM then exiting settings menu.
void RemoteSettings::saveToEEPROM()
{
  this->settingsVersion = SETTINGS_VERSION_CHECK;
  EEPROM.put(0, *this);
}

int RemoteSettings::getSettingValue(int index)
{
  int value;
  switch (index)
  {
  case 0: value = this->rotateDisplay;               break;
  case 1: value = this->barShowsInput;               break;
  case 2: value = this->batteryType;                 break;
  case 3: value = this->batteryCells;                break;
  case 4: value = this->throttleDeadzone;            break;
  case 5: value = this->minHallValue;                break;
  case 6: value = this->centerHallValue;             break;
  case 7: value = this->maxHallValue;                break;
  case 8: value = this->brakeEndpoint;               break;
  case 9: value = this->throttleEndpoint;            break;
  case 10: value = this->brakeAccelerationTime;      break;
  case 11: value = this->throttleAccelerationTime;   break;
  case 12: value = this->cruiseAccelerationTime;     break;
  }
  return value;
}

void RemoteSettings::setSettingValue(int index, int value)
{
  switch (index)
  {
  case 0: this->rotateDisplay = constrain(value, SETTINGS_RULES[index][1], SETTINGS_RULES[index][2]);               break;
  case 1: this->barShowsInput = constrain(value, SETTINGS_RULES[index][1], SETTINGS_RULES[index][2]);               break;
  case 2: this->batteryType = constrain(value, SETTINGS_RULES[index][1], SETTINGS_RULES[index][2]);                 break;
  case 3: this->batteryCells = constrain(value, SETTINGS_RULES[index][1], SETTINGS_RULES[index][2]);                break;
  case 4: this->throttleDeadzone = constrain(value, SETTINGS_RULES[index][1], SETTINGS_RULES[index][2]);            break;
  case 5: this->minHallValue = constrain(value, SETTINGS_RULES[index][1], SETTINGS_RULES[index][2]);                break;
  case 6: this->centerHallValue = constrain(value, SETTINGS_RULES[index][1], SETTINGS_RULES[index][2]);             break;
  case 7: this->maxHallValue = constrain(value, SETTINGS_RULES[index][1], SETTINGS_RULES[index][2]);                break;
  case 8: this->brakeEndpoint = constrain(value, SETTINGS_RULES[index][1], SETTINGS_RULES[index][2]);               break;
  case 9: this->throttleEndpoint = constrain(value, SETTINGS_RULES[index][1], SETTINGS_RULES[index][2]);            break;
  case 10: this->brakeAccelerationTime = constrain(value, SETTINGS_RULES[index][1], SETTINGS_RULES[index][2]);      break;
  case 11: this->throttleAccelerationTime = constrain(value, SETTINGS_RULES[index][1], SETTINGS_RULES[index][2]);   break;
  case 12: this->cruiseAccelerationTime = constrain(value, SETTINGS_RULES[index][1], SETTINGS_RULES[index][2]);     break;
  }
}

void RemoteSettings::increaseSetting(int index)
{
  switch (index)
  {
  case 0:
    this->rotateDisplay = !this->rotateDisplay;
    break;
  case 1:
    this->barShowsInput = !this->barShowsInput;
    break;
  case 2:
  case 3:
  case 4:
  {
    int val = this->getSettingValue(index) + 1;
    this->setSettingValue(index, val);
    break;
  }
  case 5:
  case 6:
  case 7:
  {
    int val = this->getSettingValue(index) + 10;
    this->setSettingValue(index, val);
    break;
  }

  case 8:
  case 9:
  {
    int val = this->getSettingValue(index) + 1;
    this->setSettingValue(index, val);
    break;
  }

  case 10:
  {
    float val = this->brakeAccelerationTime + 0.1;
    this->brakeAccelerationTime = constrain(val, SETTINGS_RULES[index][1], SETTINGS_RULES[index][2]);
    break;
  }
  case 11:
  {
    float val = this->throttleAccelerationTime + 0.1;
    this->throttleAccelerationTime = constrain(val, SETTINGS_RULES[index][1], SETTINGS_RULES[index][2]);
    break;
  }
  case 12:
  {
    float val = this->cruiseAccelerationTime + 0.1;
    this->cruiseAccelerationTime = constrain(val, SETTINGS_RULES[index][1], SETTINGS_RULES[index][2]);
    break;
  }
  }
}

void RemoteSettings::decreaseSetting(int index)
{
  switch (index)
  {
  case 0:
    this->rotateDisplay = !this->rotateDisplay;
    break;
  case 1:
    this->barShowsInput = !this->barShowsInput;
    break;
  case 2:
  case 3:
  case 4:
  {
    int val = this->getSettingValue(index) - 1;
    this->setSettingValue(index, val);
    break;
  }
  case 5:
  case 6:
  case 7:
  {
    int val = this->getSettingValue(index) - 10;
    this->setSettingValue(index, val);
    break;
  }

  case 8:
  case 9:
  {
    int val = this->getSettingValue(index) - 1;
    this->setSettingValue(index, val);
    break;
  }

  case 10:
  {
    float val = this->brakeAccelerationTime - 0.1;
    this->brakeAccelerationTime = constrain(val, SETTINGS_RULES[index][1], SETTINGS_RULES[index][2]);
    break;
  }
  case 11:
  {
    float val = this->throttleAccelerationTime - 0.1;
    this->throttleAccelerationTime = constrain(val, SETTINGS_RULES[index][1], SETTINGS_RULES[index][2]);
    break;
  }
  case 12:
  {
    float val = this->cruiseAccelerationTime - 0.1;
    this->cruiseAccelerationTime = constrain(val, SETTINGS_RULES[index][1], SETTINGS_RULES[index][2]);
    break;
  }
  }
}

String RemoteSettings::getSettingString(int index)
{
  switch (index)
  {
  case 0: return String(F("Rotate Display"));
  case 1: return String(F("Bar shows input"));
  case 2: return String(F("Battery type"));
  case 3: return String(F("Battery cells"));
  case 4: return String(F("Throttle deadzone"));
  case 5: return String(F("Throttle min"));
  case 6: return String(F("Throttle center"));
  case 7: return String(F("Throttle max"));
  case 8: return String(F("Brake endpoint"));
  case 9: return String(F("Thr. endpoint"));
  case 10: return String(F("Brake accel."));
  case 11: return String(F("Throttle accel."));
  case 12: return String(F("Cruise accel."));
  case 13: return String(F("RESET ALL"));
  }
}

String RemoteSettings::getSettingStringUnit(int index)
{
  switch (index)
  {
  case 0: return String();
  case 1: return String();
  case 2: return String();
  case 3: return String(F("S"));
  case 4: return String(F("%"));
  case 5: return String();
  case 6: return String();
  case 7: return String();
  case 8: return String(F("%"));
  case 9: return String(F("%"));
  case 10: return String(F(" sec"));
  case 11: return String(F(" sec"));
  case 12: return String(F(" sec"));
  case 13: return String();
  }
}


String RemoteSettings::getSettingValueString(int index)
{
  switch (index)
  {
  case 0: return this->rotateDisplay ? String(F("True")) : String(F("False"));
  case 1: return this->barShowsInput ? String(F("True")) : String(F("False"));
  case 2: return this->batteryType ? String(F("LiIon")) : String(F("LiPo"));
  case 3: return String(this->batteryCells);
  case 4: return String(int(this->throttleDeadzone));
  case 5: return String(this->minHallValue);
  case 6: return String(this->centerHallValue);
  case 7: return String(this->maxHallValue);
  case 8: return String(this->brakeEndpoint);
  case 9: return String(this->throttleEndpoint);
  case 10: return String(this->brakeAccelerationTime);
  case 11: return String(this->throttleAccelerationTime);
  case 12: return String(this->cruiseAccelerationTime);
  case 13: return String(F("Confirm"));
  }
  return String(F("Unknown"));
}


bool RemoteSettings::isThrottleHallSetting(int index)
{
  switch (index)
  {
  case 5:
  case 6:
  case 7:
    return true;
  default:
    return false;
  }

  return false;
}

bool RemoteSettings::inRange(int val, byte settingIndex)
{
  return ((SETTINGS_RULES[settingIndex][1] <= val) && (val <= SETTINGS_RULES[settingIndex][2]));
}
