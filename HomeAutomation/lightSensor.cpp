// Loading the required libraries
#include <PubSubClient.h>
#include "lightSensor.h"

LightSensor::LightSensor(int lightSensorPin, int threshold, String topicPrefix)
{
  // Topics
  baseTopic = topicPrefix + "/light";
  lightValueTopic = baseTopic + "/value";
  lightThresholdTopic = baseTopic + "/currentThreshold";

  this->isArmed = true;

  this->lightPin = lightSensorPin;
  this->lightThreshold = threshold;
  this->lightSensorTime = 300;
  
  this->setup();
}

// The setup function sets your ESP GPIOs to Outputs, starts the serial communication at a baud rate of 115200
// Sets your mqtt broker and sets the callback function
// The callback function is what receives messages and actually controls the LEDs
void LightSensor::setup() {
  if (!this->isArmed) 
  {
    return;
  }
  
  pinMode(this->lightPin, INPUT);
  this->lastLightValue = -10000;
}

void LightSensor::loop(PubSubClient& client) {
  if (!this->isArmed)
  {
    return;
  }
  
  long now = millis();
  if (now - this->lastLightMeasureTime > this->lightSensorTime) {
    this->lastLightMeasureTime = now;
    process(client);
  }  
}

void LightSensor::Arm()
{
  this->isArmed = true;  
}

void LightSensor::Disarm()
{
  this->isArmed = false;
}

bool LightSensor::IsArmed()
{
  return this->isArmed;
}

void LightSensor::process(PubSubClient& client)
{
  if (!this->isArmed)
  {
    return;
  }
  
  int light = analogRead(this->lightPin);
  int m = map(light, 0, 330, 0, 100);

  // Publish only if the value changed
  if (this->lastLightValue <= 0 || (m - this->lastLightValue) != 0)
  {
    Serial.print("Raw light read: ");
    Serial.print(light);
    Serial.print("  Normalized: ");
    Serial.println(m);

    this->lastLightValue = m;
    this->ReportValue(client);   
  }
}

void LightSensor::ReportThreshold(PubSubClient& client)
{
  if (!this->isArmed)
  {
    return;
  }
  
  static char lightValTemp[6];
  itoa(this->lightThreshold, lightValTemp, 10);
  client.publish(lightThresholdTopic.c_str(), lightValTemp);
}

void LightSensor::ReportValue(PubSubClient& client)
{
  if (!this->isArmed)
  {
    return;
  }
  
  static char lightValTemp[6];
  itoa(this->lastLightValue, lightValTemp, 10);
  client.publish(lightValueTopic.c_str(), lightValTemp);
}

int LightSensor::SetThreshold(int newThreshold)
{
  if (newThreshold >= 0 && newThreshold <= 100)
  {
    this->lightThreshold = newThreshold;
  }

  return this->lightThreshold;
} 
   
float LightSensor::GetLightValue()
{
  return this->lastLightValue;
}

bool LightSensor::IsOverThreshold()
{
  return this->isArmed ? this->lightThreshold < this->lastLightValue : false;
}

bool LightSensor::IsOverThreshold(float gap)
{
  return this->isArmed ? this->lightThreshold * gap < this->lastLightValue : false;
}

