// Loading the required libraries
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include "SmokeSensor.h"

Smoke::Smoke(int smokePin, int smokeLEDPin, int buzzerPin)
{
  // Smoke data input (A/D converter input)
  this->smokePin = smokePin < 0 ? A0 : smokePin;
  
  // Status LEDs (output)
  this->smokeLEDPin = smokeLEDPin < 0 ? 13 : smokeLEDPin;

  // Buzzer pin (output)
  this->buzzerPin = buzzerPin < 0 ? 14 : buzzerPin;

  // Smoke Threshold
  this->smokeThres = 70;

  // Control Variables
  this->smokeTriggered = false;
  this->testMode = false;

  // Blink control variables
  this->blinkFlag = false;
  this->blinkStatus = true;
  this->blinkTime = 500;
  this->lastBlinkCheck = 0;

  // Measurement control variables
  this->lastMeasure = 0;
  this->lastSmokeCheck = 0;

  // Test mode control variables
  this->testModeStart = 0;
}

void Smoke::setBlinkTimeInMs(int blinkTimeInMs)
{
  this->blinkTime = blinkTimeInMs < 0 ? 500 : blinkTimeInMs;
}

void Smoke::setSmokeThreshold(int smokeThreshold)
{
  this->smokeThres = smokeThreshold < 0 ? 70 : smokeThreshold;
}

void Smoke::startTestMode()
{
  this->testMode = true;
}

void Smoke::processMessage(PubSubClient &client, String topic, String messageTemp)
{
  if(topic=="home/garage/esp1/smoke"){
    Serial.print("SMOKE SENSOR STATUS CHANGE... ");
    if (messageTemp=="Test/Reset") {
      if (smokeTriggered) {
        // Reset
        smokeTriggered = false;
        testMode = false;
        blinkFlag = false;
        blinkStatus = false;
        client.publish("home/garage/esp1/smoke/notification", "NO SMOKE");
        Serial.println("NO SMOKE");
      }
      else 
      {
        // Test
        smokeTriggered = false;
        testMode = true;
        blinkFlag = true;
        blinkStatus = true;
        testModeStart = millis();
        lastBlinkCheck = testModeStart;
        client.publish("home/garage/esp1/smoke/notification", "IN TEST");
        Serial.println("IN TEST");
      }

      this->blink(blinkStatus);
    }
  }  
}

void Smoke::reportStatus(PubSubClient *client, boolean smokeTriggered)
{
  if (!client) 
  {
    client->publish("home/garage/esp1/smoke/notification", "NOT AVAILABLE");
    return;
  }
  
  if (smokeTriggered) {
    client->publish("home/garage/esp1/smoke/notification", "SMOKE DETECTED!!!");
    Serial.println("SMOKE DETECTED!!!");
  }
  else
  {
    // Not in test mode
    client->publish("home/garage/esp1/smoke/notification", "NO SMOKE");
    Serial.println("NO SMOKE");
  }  
}

void Smoke::blink(boolean blinkStatus)
{
  if (blinkStatus) {
    digitalWrite(smokeLEDPin, HIGH);
    tone(buzzerPin, 3400);
  }
  else
  {
    digitalWrite(smokeLEDPin, LOW);
    noTone(buzzerPin);
  }  
}

void Smoke::subscribe(PubSubClient &client)
{
   client.subscribe("home/garage/esp1/smoke");
}

void Smoke::setup()
{
  pinMode(smokeLEDPin, OUTPUT);
  pinMode(buzzerPin, OUTPUT);
  pinMode(smokePin, INPUT);
}

void Smoke::check(PubSubClient* client)
{
  long now = millis();
  
  // Checks smoke
  if (testMode) {
    // Timer reached? (5 sec in test mode)
    if (now - testModeStart > 5000)
    {
      // Stop blink and buzz
      blinkFlag = false;
      blinkStatus = false;
      this->blink(blinkStatus);
      testMode = false;
    }
    else if (now - lastBlinkCheck > blinkTime)
    {
      // Blink every second
      blinkStatus = !blinkStatus;
      this->blink(blinkStatus);
      lastBlinkCheck = now;
    }
  }
  else if (now - lastSmokeCheck > 2000) {
    // Read smoke after 2 secs
    lastSmokeCheck = now;
    int smokeValue = analogRead(smokePin);
    Serial.print("Smoke value: ");
    Serial.println(smokeValue);

    smokeTriggered = smokeValue > smokeThres;
    if (smokeTriggered) {
      // Start blinking if not already doing it, if blinking already flip if the timer has been reached
      if (blinkFlag)
      {
        if (now - lastBlinkCheck > blinkTime)
        {
          blinkStatus = !blinkStatus;
          this->blink(blinkStatus);
          lastBlinkCheck = now;
        }
      }
      else
      {
        blinkFlag = true;
        blinkStatus = true;
        this->blink(blinkStatus);
        lastBlinkCheck = now;
      }
    }
    else
    {
      blinkFlag = false;
      blinkStatus = false;
      this->blink(blinkStatus);
    }

    reportStatus(client, smokeTriggered);
  }  
}

void Smoke::loop(PubSubClient* client)
{
  check(client);
  reportStatus(client, false);
}

