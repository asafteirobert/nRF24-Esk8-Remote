#include "Constants.h"
#include "RemoteSettings.h"
#include <U8g2lib.h>
#include <Wire.h>
#include <SPI.h>
#include "RF24.h"
#include "VescUart.h"

// #define DEBUG

#ifdef DEBUG
#define DEBUG_PRINT(x)  Serial.println (x)
#include "printf.h"
#else
#define DEBUG_PRINT(x)
#endif

// Defining the type of display used (128x32)
U8G2_SSD1306_128X32_UNIVISION_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);

//Telemetry data returned from the receiver
struct returnDataType
{
  float sensorVoltage = 0;
};

byte currentSetting = 0;

struct returnDataType returnData;
struct RemoteSettings remoteSettings;


// Defining variables for Hall Effect throttle.
short hallMeasurement, throttle;

// Defining variables for NRF24 communication
bool connected = false;
short failCount;
unsigned long lastTransmission;

// Defining variables for OLED display
char displayBuffer[20];
short displayedPage = 0;
bool signalBlink = false;
unsigned long lastSignalBlink;
unsigned long lastDataRotation;

// RF24 object for NRF24 communication
RF24 radio(PIN_NRF_CE, PIN_NRF_CS);

// Defining variables for Settings menu
bool changeSettings = false;
bool changeSelectedSetting = false;

bool settingsLoopFlag = false;
bool settingsChangeFlag = false;
bool settingsChangeValueFlag = false;

void setup()
{
#ifdef DEBUG
  Serial.begin(9600);
#endif

  remoteSettings.loadFromEEPROM();

  pinMode(PIN_TRIGGER, INPUT_PULLUP);
  pinMode(PIN_HALL_SENSOR, INPUT);
  pinMode(PIN_BATTERY_MEASURE, INPUT);

  u8g2.begin();

  drawStartScreen();

  if (triggerActive())
  {
    changeSettings = true;
    drawTitleScreen(F("Remote Settings"));
  }

  // Start radio communication
  radio.begin();
  radio.setPALevel(RF24_PA_MAX);
  radio.setDataRate(RF24_250KBPS);
  radio.enableAckPayload();
  radio.enableDynamicPayloads();
  radio.openWritingPipe(NRF_PIPE);

#ifdef DEBUG
  printf_begin();
  radio.printDetails();
#endif
}

void loop()
{
  calculateThrottlePosition();

  if (changeSettings == true)
  {
    // Use throttle and trigger to change settings
    controlSettingsMenu();
  }
  else
  {
    if (!triggerActive() && throttle>127)
      throttle = 127;

    // Transmit to receiver
    transmitThrottle();
  }

  // Update display and LED
  updateMainDisplay();
}

void controlSettingsMenu()
{
  if (triggerActive())
  {
    if (settingsChangeFlag == false)
    {

      // Save settings to EEPROM
      if (changeSelectedSetting == true)
      {
        remoteSettings.saveToEEPROM();
      }

      changeSelectedSetting = !changeSelectedSetting;
      settingsChangeFlag = true;
    }
  }
  else
  {
    settingsChangeFlag = false;
  }

  if (hallMeasurement >= (remoteSettings.maxHallValue - 150) && settingsLoopFlag == false)
  {
    // Up
    if (changeSelectedSetting == true)
    {
      int val = remoteSettings.getSettingValue(currentSetting) + 1;

      if (remoteSettings.inRange(val, currentSetting))
      {
        remoteSettings.setSettingValue(currentSetting, val);
        settingsLoopFlag = true;
      }
    }
    else
    {
      if (currentSetting != 0)
      {
        currentSetting--;
        settingsLoopFlag = true;
      }
    }
  }
  else if (hallMeasurement <= (remoteSettings.minHallValue + 150) && settingsLoopFlag == false)
  {
    // Down
    if (changeSelectedSetting == true)
    {
      int val = remoteSettings.getSettingValue(currentSetting) - 1;

      if (remoteSettings.inRange(val, currentSetting))
      {
        remoteSettings.setSettingValue(currentSetting, val);
        settingsLoopFlag = true;
      }
    }
    else
    {
      if (currentSetting < (SETTINGS_COUNT - 1))
      {
        currentSetting++;
        settingsLoopFlag = true;
      }
    }
  }
  
  else if ((remoteSettings.centerHallValue - 50 <= hallMeasurement) && (hallMeasurement <= remoteSettings.centerHallValue + 50))
  {
    settingsLoopFlag = false;
  }
}

void drawSettingNumber()
{
  // Position on OLED
  int x = 2; int y = 10;

  // Draw current setting number box
  u8g2.drawRFrame(x + 102, y - 10, 22, 32, 4);

  // Draw current setting number
  String displayString = (String)(currentSetting + 1);
  displayString.toCharArray(displayBuffer, displayString.length() + 1);

  u8g2.setFont(u8g2_font_profont22_tn);
  u8g2.drawStr(x + 108, 22, displayBuffer);
}

void drawSettingsMenu()
{
  // Position on OLED
  int x = 0; int y = 10;

  // Draw setting title
  String displayString = SETTINGS_NAMES[currentSetting][0];
  displayString.toCharArray(displayBuffer, displayString.length() + 1);

  u8g2.setFont(u8g2_font_profont12_tr);
  u8g2.drawStr(x, y, displayBuffer);

  int val = remoteSettings.getSettingValue(currentSetting);

  displayString = (String)val + "" + SETTINGS_NAMES[currentSetting][1];
  displayString.toCharArray(displayBuffer, displayString.length() + 1);
  u8g2.setFont(u8g2_font_10x20_tr);

  if (changeSelectedSetting == true)
  {
    u8g2.drawStr(x + 10, y + 20, displayBuffer);
  }
  else
  {
    u8g2.drawStr(x, y + 20, displayBuffer);
  }
}

// Return true if trigger is activated, false otherwice
boolean triggerActive()
{
    return digitalRead(PIN_TRIGGER) == LOW;
}

// Function used to transmit the throttle value, and receive the telemetry data.
void transmitThrottle()
{
  // Transmit once every 19 millisecond
  if (millis() - lastTransmission >= 19)
  {
    lastTransmission = millis();

    boolean sendSuccess = false;
    // Transmit the speed value (0-255).
    sendSuccess = radio.write(&throttle, sizeof(throttle));

    // Listen for an acknowledgement reponse (return of VESC data).
    while (radio.isAckPayloadAvailable())
    {
      radio.read(&returnData, sizeof(returnData));
    }

    if (sendSuccess == true)
    {
      // Transmission was a succes
      failCount = 0;
      sendSuccess = false;

      DEBUG_PRINT(F("Transmission succes"));
    }
    else
    {
      // Transmission was not a succes
      failCount++;

      DEBUG_PRINT(F("Failed transmission"));
    }

    // If lost more than 5 transmissions, we can assume that connection is lost.
    if (failCount < 5)
      connected = true;
    else
      connected = false;
  }
}

void calculateThrottlePosition()
{
  // Hall sensor reading can be noisy, lets make an average reading.
  int total = 0;
  for (int i = 0; i < 10; i++)
  {
    total += analogRead(PIN_HALL_SENSOR);
  }
  hallMeasurement = total / 10;

  DEBUG_PRINT((String)hallMeasurement);

  if (hallMeasurement >= remoteSettings.centerHallValue)
    throttle = constrain(map(hallMeasurement, remoteSettings.centerHallValue, remoteSettings.maxHallValue, 127, 255), 127, 255);
  else
    throttle = constrain(map(hallMeasurement, remoteSettings.minHallValue, remoteSettings.centerHallValue, 0, 127), 0, 127);

  // removeing center noise
  if (abs(throttle - 127) < THROTTLE_DEADZONE)
    throttle = 127;
}

// Function used to indicate the remotes battery level.
int batteryLevel()
{
  float voltage = batteryVoltage();

  if (voltage <= REMOTE_BATTERY_MIN_VOLTAGE)
    return 0;
  else if (voltage >= REMOTE_BATTERY_MAX_VOLTAGE)
    return 100;
  else
    return (voltage - REMOTE_BATTERY_MIN_VOLTAGE) * 100 / (REMOTE_BATTERY_MAX_VOLTAGE - REMOTE_BATTERY_MIN_VOLTAGE);
}

// Function to calculate and return the remote's battery voltage.
float batteryVoltage()
{
  analogReference(INTERNAL);
  analogRead(PIN_BATTERY_MEASURE);
  delay(5);
  analogRead(PIN_BATTERY_MEASURE);

  int total = 0;
  for (int i = 0; i < 10; i++)
    total += analogRead(PIN_BATTERY_MEASURE);

  analogReference(DEFAULT);
  analogRead(PIN_HALL_SENSOR);
  delay(1);

  return (REMOTE_BATTERY_SENSOR_REF_VOLTAGE / 1024.0) * ((float)total / 10.0) * REMOTE_BATTERY_SENSOR_MULTIPLIER;
}

void updateMainDisplay()
{
  u8g2.clearBuffer();

  if (changeSettings == true)
  {
    drawSettingsMenu();
    drawSettingNumber();
  }
  else
  {
    drawThrottle();
    drawPage();
    drawBatteryLevel();
    drawSignal();
  }

  u8g2.sendBuffer();
}

void drawStartScreen()
{
  u8g2.clearBuffer();

  u8g2.drawXBMP(4, 4, 24, 24, ICON_LOGO);

  String displayString = F("Esk8 remote");
  displayString.toCharArray(displayBuffer, 12);
  u8g2.setFont(u8g2_font_helvR10_tr);
  u8g2.drawStr(34, 22, displayBuffer);

  u8g2.sendBuffer();
  delay(1500);
}

void drawTitleScreen(String title)
{
  u8g2.clearBuffer();

  title.toCharArray(displayBuffer, 20);
  u8g2.setFont(u8g2_font_helvR10_tr);
  u8g2.drawStr(12, 20, displayBuffer);

  u8g2.sendBuffer();
  delay(1500);
}

void drawPage()
{
  int decimals;
  float value;
  String suffix;
  String prefix;

  int first, last;

  int x = 0;
  int y = 16;

  // Rotate the realtime data each 4s.
  /*
  if ((millis() - lastDataRotation) >= 4000)
  {
    lastDataRotation = millis();
    displayedPage++;

    if (displayedPage > 2)
    {
      displayedPage = 0;
    }
  }
  */

  switch (displayedPage)
  {
  case 0:
    value = returnData.sensorVoltage;
    suffix = "V";
    prefix = F("BATTERY");
    decimals = 1;
    break;
  }

  // Display prefix (title)
  String displayString = prefix;
  displayString.toCharArray(displayBuffer, 10);
  u8g2.setFont(u8g2_font_profont12_tr);
  u8g2.drawStr(x, y - 1, displayBuffer);

  // Split up the float value: a number, b decimals.
  first = abs(floor(value));
  last = value * pow(10, 3) - first * pow(10, 3);

  // Add leading zero
  if (first <= 9)
    displayString = "0" + (String)first;
  else
    displayString = (String)first;

  // Display numbers
  displayString.toCharArray(displayBuffer, 10);
  u8g2.setFont(u8g2_font_logisoso22_tn);
  u8g2.drawStr(x + 55, y + 13, displayBuffer);

  // Display decimals
  displayString = "." + (String)last;
  displayString.toCharArray(displayBuffer, decimals + 2);
  u8g2.setFont(u8g2_font_profont12_tr);
  u8g2.drawStr(x + 86, y - 1, displayBuffer);

  // Display suffix
  displayString = suffix;
  displayString.toCharArray(displayBuffer, 10);
  u8g2.setFont(u8g2_font_profont12_tr);
  u8g2.drawStr(x + 86 + 2, y + 13, displayBuffer);
}

void drawThrottle()
{
  int x = 0;
  int y = 18;

  // Draw throttle
  u8g2.drawHLine(x, y, 52);
  u8g2.drawVLine(x, y, 10);
  u8g2.drawVLine(x + 52, y, 10);
  u8g2.drawHLine(x, y + 10, 5);
  u8g2.drawHLine(x + 52 - 4, y + 10, 5);

  if (throttle >= 127)
  {
    int width = map(throttle, 127, 255, 0, 49);

    for (int i = 0; i < width; i++)
      u8g2.drawVLine(x + i + 2, y + 2, 7);
  }
  else
  {
    int width = map(throttle, 0, 126, 49, 0);
    for (int i = 0; i < width; i++)
      u8g2.drawVLine(x + 50 - i, y + 2, 7);

  }
}

void drawSignal()
{
  // Position on OLED
  int x = 114; int y = 17;

  if (connected == true)
  {
    if (triggerActive())
      u8g2.drawXBMP(x, y, 12, 12, ICON_SIGNAL_TRANSMITTING);
    else
      u8g2.drawXBMP(x, y, 12, 12, ICON_SIGNAL_CONNECTED);
  }
  else
  {
    if (millis() - lastSignalBlink > 500)
    {
      signalBlink = !signalBlink;
      lastSignalBlink = millis();
    }

    if (signalBlink == true)
      u8g2.drawXBMP(x, y, 12, 12, ICON_SIGNAL_CONNECTED);
    else
      u8g2.drawXBMP(x, y, 12, 12, ICON_SIGNAL_NOCONNECTION);
  }
}

void drawBatteryLevel()
{
  int level = batteryLevel();

  // Position on OLED
  int x = 108; int y = 4;

  u8g2.drawFrame(x + 2, y, 18, 9);
  u8g2.drawBox(x, y + 2, 2, 5);

  for (int i = 0; i < 5; i++)
  {
    int p = round((100 / 5) * i);
    if (p <= level)
      u8g2.drawBox(x + 4 + (3 * i), y + 2, 2, 5);
  }
}
