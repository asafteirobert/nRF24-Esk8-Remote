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
  EEPROM.put(0, *this);
}

int RemoteSettings::getSettingValue(int index)
{
  int value;
  switch (index)
  {
  case 0: value = this->triggerMode;     break;
  case 1: value = this->batteryType;     break;
  case 2: value = this->batteryCells;    break;
  case 3: value = this->motorPoles;      break;
  case 4: value = this->motorPulley;     break;
  case 5: value = this->wheelPulley;     break;
  case 6: value = this->wheelDiameter;   break;
  case 7: value = this->useUart;         break;
  case 8: value = this->minHallValue;    break;
  case 9: value = this->centerHallValue; break;
  case 10: value = this->maxHallValue;   break;
  }
  return value;
}

void RemoteSettings::setSettingValue(int index, int value)
{
  switch (index)
  {
  case 0: this->triggerMode = value;     break;
  case 1: this->batteryType = value;     break;
  case 2: this->batteryCells = value;    break;
  case 3: this->motorPoles = value;      break;
  case 4: this->motorPulley = value;     break;
  case 5: this->wheelPulley = value;     break;
  case 6: this->wheelDiameter = value;   break;
  case 7: this->useUart = value;         break;
  case 8: this->minHallValue = value;    break;
  case 9: this->centerHallValue = value; break;
  case 10: this->maxHallValue = value;   break;
  }
}


bool RemoteSettings::inRange(int val, byte settingIndex)
{
  return ((SETTINGS_RULES[settingIndex][1] <= val) && (val <= SETTINGS_RULES[settingIndex][2]));
}
