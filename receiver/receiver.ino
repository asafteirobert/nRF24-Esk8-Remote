#include <SPI.h>
#include <nRF24L01.h>
#include <Servo.h>
#include "RF24.h"
//#include "VescUart.h"

struct returnDataType
{
  float sensorVoltage = 0;
};

const uint64_t NRF_PIPE = 0xE8E8F0F0E1LL;
const int timeoutMax = 500;
const int outputPin1 = 5;
const int outputPin2 = 6;
const int voltageSensorPin = A1;
const float voltageSensorReference = 1.089; //Slightly lower than the expected 1v1 
const float voltageSensorMultiplier = 48; //Voltage divider R1=47k, R2=1K

RF24 radio(9, 10);
Servo servoOutput1;
Servo servoOutput2;

bool recievedData = false;
uint32_t lastTimeReceived = 0;
float outputValue = 0.5;
//unsigned long lastDataCheck;


//struct bldcMeasure measuredValues;
struct returnDataType returnData;


void setup()
{
  //Serial.begin(115200);

  radio.begin();
  radio.setPALevel(RF24_PA_MAX);
  radio.setDataRate(RF24_250KBPS);
  radio.enableAckPayload();
  radio.enableDynamicPayloads();
  radio.openReadingPipe(1, NRF_PIPE);
  radio.startListening();

  servoOutput1.attach(outputPin1);
  servoOutput2.attach(outputPin2);

  pinMode(voltageSensorPin, INPUT);
  analogReference(INTERNAL);
  analogRead(voltageSensorPin);

}

void loop()
{
  //average across a long time to avoid jitter
  returnData.sensorVoltage = returnData.sensorVoltage * 0.9 + 0.1 * readSensorVoltage();

  // If transmission is available
  if (radio.available())
  {
    // Send back telemetry
    radio.writeAckPayload(NRF_PIPE, &returnData, sizeof(returnData));

    // Read the actual message
    radio.read(&outputValue, sizeof(outputValue));
    recievedData = true;
  }

  if (recievedData == true)
  {
    // A speed is received from the transmitter (remote).
    lastTimeReceived = millis();
    recievedData = false;
  }
  else if ((millis() - lastTimeReceived) > timeoutMax)
  {
    // No speed is received within the timeout limit.
    outputValue = 0.5;
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
    total += analogRead(voltageSensorPin);
  return (voltageSensorReference / 1024.0) * ((float)total / 10.0) * voltageSensorMultiplier;
}

float mapfloat(float x, float in_min, float in_max, float out_min, float out_max)
{
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}
