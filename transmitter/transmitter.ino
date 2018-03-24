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
float hallMeasurement;
float throttle = 0.5;
unsigned long lastThrottleMeasurement = 0;

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
bool batteryBlink = false;
unsigned long lastBatteryBlink;


// RF24 object for NRF24 communication
RF24 radio(PIN_NRF_CE, PIN_NRF_CS);

// Defining variables for Settings menu
bool changeSettings = false;
bool settingsLoopFlag = false;
bool settingsChangeFlag = false;
bool settingsChangeValueFlag = false;

//cruise control variables
bool cruiseControlActive = false;
float cruiseControlTarget = 0.6;

void setup()
{
#ifdef DEBUG
  Serial.begin(9600);
#endif

  remoteSettings.loadFromEEPROM();
  if (remoteSettings.rotateDisplay)
    u8g2.setDisplayRotation(U8G2_R2);

  pinMode(PIN_TRIGGER, INPUT_PULLUP);
  pinMode(PIN_PAGE_SWITCH, INPUT_PULLUP);
  pinMode(PIN_CRUISE_CONTROL, INPUT_PULLUP);
  pinMode(PIN_HALL_SENSOR, INPUT);
  pinMode(PIN_BATTERY_MEASURE, INPUT);

  u8g2.begin();

  drawStartScreen();

  if (isTriggerButtonActive())
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
    if (!isTriggerButtonActive() && throttle>0.5)
      throttle = 0.5;

    // Transmit to receiver
    transmitThrottle();
  }

  // Update display and LED
  updateMainDisplay();
}

void controlSettingsMenu()
{
  if (isPageSwitchButtonActive())
  {
    if (settingsChangeFlag == false)
    {
      remoteSettings.saveToEEPROM();
      currentSetting++;
      if (currentSetting >= SETTINGS_COUNT)
        currentSetting = 0;
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
    remoteSettings.increaseSetting(currentSetting);
    settingsLoopFlag = true;
  }
  else if (hallMeasurement <= (remoteSettings.minHallValue + 150) && settingsLoopFlag == false)
  {
    // Down
    remoteSettings.decreaseSetting(currentSetting);
    settingsLoopFlag = true;
  }
  else if (isCruiseControlButtonActive())
  {
    if (remoteSettings.isThrottleHallSetting(currentSetting))
      remoteSettings.setSettingValue(currentSetting, hallMeasurement);
    if (currentSetting == SETTINGS_COUNT - 1 && settingsLoopFlag == false)
    {
      remoteSettings.resetToDefault();
      remoteSettings.saveToEEPROM();
      currentSetting = 0;
    }

    settingsLoopFlag = true;
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
  if (currentSetting < 9)
    u8g2.drawRFrame(x + 102, y - 10, 22, 32, 4);
  else
    u8g2.drawRFrame(x + 92, y - 10, 32, 32, 4);

  // Draw current setting number
  String displayString = (String)(currentSetting + 1);
  displayString.toCharArray(displayBuffer, displayString.length() + 1);

  u8g2.setFont(u8g2_font_profont22_tn);
  if (currentSetting < 9)
    u8g2.drawStr(x + 108, 22, displayBuffer);
  else
    u8g2.drawStr(x + 98, 22, displayBuffer);
}

void drawSettingsMenu()
{
  // Position on OLED
  int x = 0; int y = 10;

  // Draw setting title
  String displayString = remoteSettings.getSettingString(currentSetting);
  displayString.toCharArray(displayBuffer, displayString.length() + 1);

  u8g2.setFont(u8g2_font_profont12_tr);
  u8g2.drawStr(x, y, displayBuffer);

  displayString = remoteSettings.getSettingValueString(currentSetting) + remoteSettings.getSettingStringUnit(currentSetting);
  displayString.toCharArray(displayBuffer, displayString.length() + 1);
  u8g2.setFont(u8g2_font_10x20_tr);

  u8g2.drawStr(x, y + 20, displayBuffer);
}

bool isTriggerButtonActive()
{
  return digitalRead(PIN_TRIGGER) == LOW;
}

bool isPageSwitchButtonActive()
{
  return digitalRead(PIN_PAGE_SWITCH) == LOW;
}

bool isCruiseControlButtonActive()
{
  return digitalRead(PIN_CRUISE_CONTROL) == LOW;
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
  //TODO: hall sensor calibration for low battery voltages
  for (int i = 0; i < 10; i++)
  {
    total += analogRead(PIN_HALL_SENSOR);
  }
  hallMeasurement = (float)total / 10;

  float desiredThrottleFromSensor;
  float initialThrottle = throttle;

  if (hallMeasurement >= remoteSettings.centerHallValue)
    desiredThrottleFromSensor = ((constrain(mapfloat(hallMeasurement, remoteSettings.centerHallValue, remoteSettings.maxHallValue, 0.5, 1), 0.5, 1) - 0.5) * remoteSettings.throttleEndpoint / 100) + 0.5;
  else
    desiredThrottleFromSensor = 0.5 - ((0.5 - constrain(mapfloat(hallMeasurement, remoteSettings.minHallValue, remoteSettings.centerHallValue, 0, 0.5), 0, 0.5)) * remoteSettings.brakeEndpoint / 100);

  // removing center noise
  if (abs(desiredThrottleFromSensor - 0.5) <= remoteSettings.throttleDeadzone / 100 / 2)
    desiredThrottleFromSensor = 0.5;

  throttle = desiredThrottleFromSensor;

  //alter throttle based on acceleration logic
  unsigned long microsNow = micros();
  float timeDifference = microsNow - lastThrottleMeasurement;
  lastThrottleMeasurement = microsNow;

  float difference = desiredThrottleFromSensor - initialThrottle;
  if (difference > 0 && remoteSettings.throttleAccelerationTime > 0) //accelerating
  {
    float maxThrottleChange = timeDifference / remoteSettings.throttleAccelerationTime / 1000000 / 2;
    if (difference > maxThrottleChange)
    {
      if (initialThrottle > 0.5)
        throttle = initialThrottle + maxThrottleChange;
      else
        throttle = min(desiredThrottleFromSensor, 0.5 + maxThrottleChange);
    }
  }
  else if (difference < 0 && remoteSettings.brakeAccelerationTime > 0) //braking
  {
    float maxThrottleChange = timeDifference / remoteSettings.brakeAccelerationTime / 1000000 / 2;
    if (-difference > maxThrottleChange)
    {
      if (initialThrottle < 0.5)
        throttle = initialThrottle - maxThrottleChange;
      else
        throttle = max(desiredThrottleFromSensor, 0.5 - maxThrottleChange);
    }
  }

  //alter throttle based on cruise control
  if (throttle >= 0.5) //ignore any CC if braking
  {
    if (isCruiseControlButtonActive())
    {
      if (!cruiseControlActive) //first time pressing CC
      {
        cruiseControlActive = true;
        //reset CC to a new value if we have throttle, otherwise use previous value
        if (throttle > 0.5)
          cruiseControlTarget = throttle;
      }

      //when trigger throttle is above CC target, use throttle instead, for extra power
      if (throttle < cruiseControlTarget)
      {
        //accelerate towards target
        float difference = cruiseControlTarget - initialThrottle;
        if (remoteSettings.cruiseAccelerationTime > 0)
        {
          float maxThrottleChange = timeDifference / remoteSettings.cruiseAccelerationTime / 1000000 / 2;
          if (difference > maxThrottleChange)
            if (initialThrottle > 0.5)
              throttle = initialThrottle + maxThrottleChange;
            else
              throttle = min(cruiseControlTarget, 0.5 + maxThrottleChange);
          else
            throttle = cruiseControlTarget;
        }
        else 
          throttle = cruiseControlTarget;
      }

    }
    else
      cruiseControlActive = false;
  }

  DEBUG_PRINT(String(hallMeasurement) + "  " + String(throttle));

  //final sanity check
  throttle = constrain(throttle, 0, 1);
}

// Function to calculate and return the remote's battery voltage.
float batteryVoltage()
{
  //TODO: because of the delays, only calculate this once a second
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

  if (throttle >= 0.5)
  {
    int width = mapfloat(throttle, 0.5, 1, 0, 49);

    for (int i = 0; i < width; i++)
      u8g2.drawVLine(x + i + 2, y + 2, 7);
  }
  else
  {
    int width = mapfloat(throttle, 0, 0.5, 49, 0);
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
    if (isTriggerButtonActive())
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
  float level = calculateBatteryLevel(batteryVoltage(), REMOTE_BATTERY_TYPE);

  // Position on OLED
  int x = 108; int y = 4;
  if (level > 1)
  {
    u8g2.drawFrame(x + 2, y, 18, 9);
    u8g2.drawBox(x, y + 2, 2, 5);

    byte bars = truncf(level / (100.0 / 6));
    for (int i = 0; (i < 5 && i < bars); i++)
      u8g2.drawBox(x + 4 + (3 * i), y + 2, 2, 5);
  }
  else
  {
    if (millis() - lastBatteryBlink > 500)
    {
      batteryBlink = !batteryBlink;
      lastBatteryBlink = millis();
    }

    if (batteryBlink == true)
    {
      u8g2.drawFrame(x + 2, y, 18, 9);
      u8g2.drawBox(x, y + 2, 2, 5);
    }
  }
}

float mapfloat(float x, float in_min, float in_max, float out_min, float out_max)
{
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

//return percent from voltage. Type is 0 for Lipo, 1 for Li-ion
float calculateBatteryLevel(float volatge, byte type)
{
  int i;
  float dx, dy;
  type = type + 1; //index 1 for lipo, index 2 for liion

  // volatge is less than the minimum
  if (volatge < BATTERY_LEVEL_TABLE[0][0])
    return BATTERY_LEVEL_TABLE[0][type];

  //voltage more than maximum
  if (volatge > BATTERY_LEVEL_TABLE[BATTERY_LEVEL_TABLE_COUNT - 1][0])
    return BATTERY_LEVEL_TABLE[BATTERY_LEVEL_TABLE_COUNT - 1][type];

  // find i, such that BATTERY_LEVEL_TABLE[i][0] <= volatge < BATTERY_LEVEL_TABLE[i+1][0]
  for (i = 0; i < BATTERY_LEVEL_TABLE_COUNT - 1; i++)
    if (BATTERY_LEVEL_TABLE[i + 1][0] > volatge)
      break;

  //interpolate
  return BATTERY_LEVEL_TABLE[i][type] +
    (volatge - BATTERY_LEVEL_TABLE[i][0]) *
    (BATTERY_LEVEL_TABLE[i + 1][type] - BATTERY_LEVEL_TABLE[i][type]) /
    (BATTERY_LEVEL_TABLE[i + 1][0] - BATTERY_LEVEL_TABLE[i][0]);
}