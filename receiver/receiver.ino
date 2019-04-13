#include <SPI.h>
#include <nRF24L01.h>
#include <Servo.h>
#include <EEPROM.h>
#include "RF24.h"
//#include "VescUart.h"

//#define DEBUG

#ifdef DEBUG
#define DEBUG_PRINT(x)  Serial.println (x)
#include "printf.h"
#else
#define DEBUG_PRINT(x)
#endif

struct ReturnDataType
{
  float sensorVoltage = 0;
};
struct BindingDataType
{
  uint8_t radioChannel;
  uint8_t uniquePipe[5];
};

const static uint8_t BINDING_PIPE[5] = { 0xE8,0xE8,0xF0,0xF0,0xE8 };
const static uint8_t BINDING_CHANNEL = 76; 

//after this timeout send failsafe throttle
const int CONNECTION_TIMEOUT = 500;
//after this timeout look for binding code again
const int REBIND_TIMEOUT = 2500;
const int PIN_OUTPUT1 = 2;
const int PIN_OUTPUT2 = 3;
const int PIN_VOLTAGE_SENSOR = A1;
const float SENSOR_REF_VOLTAGE = 1.089; //Slightly lower than the expected 1v1 
const float SENSOR_VOLTAGE_MULTIPLIER = 48; //Voltage divider R1=47k, R2=1K

RF24 radio(9, 10);
Servo servoOutput1;
Servo servoOutput2;

bool binding = true;
bool bindingState = true;
bool recievedData = false;
uint32_t lastTimeReceived = 0;
float outputValue = 0.5;
//unsigned long lastDataCheck;


//struct bldcMeasure measuredValues;
struct ReturnDataType returnData;
struct BindingDataType bindingData;
bool bindAck = true;


void setup()
{
#ifdef DEBUG
  Serial.begin(9600);
#endif

  EEPROM.get(0, bindingData);
  //make sure we dont randomly get the same pipe
  if (bindingData.uniquePipe[0] == BINDING_PIPE[0])
    bindingData.uniquePipe[0]++;

  radio.begin();
  radio.setChannel(BINDING_CHANNEL);
  radio.setPALevel(RF24_PA_MAX);
  radio.setDataRate(RF24_250KBPS);
  radio.enableAckPayload();
  radio.enableDynamicPayloads();
  radio.openReadingPipe(1, BINDING_PIPE);
  radio.openReadingPipe(2, bindingData.uniquePipe);
  radio.startListening();
  //Ack payloads are handled automatically by the radio chip when a payload is received.
  //Users should generally write an ack payload as soon as startListening() is called, so one is available when a regular payload is received. 
  radio.writeAckPayload(1, &bindAck, sizeof(bindAck));
  radio.writeAckPayload(2, &returnData, sizeof(returnData));

  servoOutput1.attach(PIN_OUTPUT1);
  servoOutput2.attach(PIN_OUTPUT2);

  pinMode(PIN_VOLTAGE_SENSOR, INPUT);
  analogReference(INTERNAL);
  analogRead(PIN_VOLTAGE_SENSOR);

#ifdef DEBUG
  printf_begin();
  radio.printDetails();
#endif
}

void loop()
{
  //average across a long time to avoid jitter
  returnData.sensorVoltage = returnData.sensorVoltage * 0.9 + 0.1 * readSensorVoltage();
  //DEBUG_PRINT("Sensor voltage: " + String(returnData.sensorVoltage));
  
  // If transmission is available
  uint8_t pipeNum;
  if (radio.available(&pipeNum))
  {
    uint8_t len = radio.getDynamicPayloadSize();
    if (pipeNum == 1 && len == sizeof(bindingData))
    {
      radio.read(&bindingData, sizeof(bindingData));
      radio.writeAckPayload(1, &bindAck, sizeof(bindAck));
      DEBUG_PRINT("Got binding data: CH" + String(bindingData.radioChannel) + " PByte" + String(bindingData.uniquePipe[0]));
      if (binding)
        bindingComplete();
      recievedData = true;
    }
    else if (pipeNum == 2 && len == sizeof(outputValue))
    {
      radio.read(&outputValue, sizeof(outputValue));
      // Send back telemetry
      radio.writeAckPayload(2, &returnData, sizeof(returnData));
      DEBUG_PRINT("output value" + String(outputValue));
      //we got a packet on the last known pipe, binding is complete
      if (binding)
        bindingComplete();
      recievedData = true;
    }
    else
    {
      radio.flush_rx();
      delay(2);
      DEBUG_PRINT("Discarded bad packet <<<");
    }

  }

  if (recievedData == true)
  {
    lastTimeReceived = millis();
    recievedData = false;
  }
  else
  {
    if ((millis() - lastTimeReceived) > CONNECTION_TIMEOUT)
    {
      outputValue = 0.5;
    }

    if ((millis() - lastTimeReceived) > REBIND_TIMEOUT)
    {
      if (binding == false)
        startBinding();
    }
  }

  if (binding)
  {
    //switch between looking for binding code or looking for actual data
    if (millis() % 600 < 300)
    {
      if (bindingState == false) //switch to binding
      {
        radio.setChannel(BINDING_CHANNEL);
        bindingState = true;
        //DEBUG_PRINT("Switched to binding");
      }
    }
    else
    {
      if (bindingState == true) //switch to data
      {
        radio.setChannel(bindingData.radioChannel);
        bindingState = false;
        //DEBUG_PRINT("Switched to data");
      }
    }
  }

  outputValue = constrain(outputValue, 0.0, 1.0);
  short pulseWidth = mapfloat(outputValue, 0, 1, 1000, 2000);
  servoOutput1.writeMicroseconds(pulseWidth);
  servoOutput2.writeMicroseconds(pulseWidth);
}

float readSensorVoltage()
{
  int total = 0;
  for (int i = 0; i < 10; i++)
    total += analogRead(PIN_VOLTAGE_SENSOR);
  return (SENSOR_REF_VOLTAGE / 1024.0) * ((float)total / 10.0) * SENSOR_VOLTAGE_MULTIPLIER;
}

void bindingComplete()
{
  binding = false;
  bindingState = false;
  radio.setChannel(bindingData.radioChannel);
  radio.closeReadingPipe(1);
  radio.closeReadingPipe(2);
  radio.flush_tx();
  delay(2);
  radio.openReadingPipe(2, bindingData.uniquePipe);
  radio.writeAckPayload(2, &returnData, sizeof(returnData));
  radio.flush_rx();
  delay(2);
  EEPROM.put(0, bindingData);
  DEBUG_PRINT("Binding complete: CH" + String(bindingData.radioChannel) + " PByte" + String(bindingData.uniquePipe[0]));
}

void startBinding()
{
  binding = true;
  bindingState = true;
  radio.setChannel(BINDING_CHANNEL);
  radio.openReadingPipe(1, BINDING_PIPE);
  radio.writeAckPayload(1, &bindAck, sizeof(bindAck));
  DEBUG_PRINT("Binding restarted");
}

float mapfloat(float x, float in_min, float in_max, float out_min, float out_max)
{
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}
