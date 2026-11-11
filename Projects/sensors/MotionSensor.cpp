// Loading the required libraries
#include <PubSubClient.h>
#include "MotionSensor.h"

Motion::Motion(int motionPin, int motionLEDPin, String topicPrefix)
{
  armMotion = false;
  motionTriggered = false;
  motionFirstTimeTriggered = false;

  // PIR Motion Sensor
  motionSensor = motionPin < 0 ? 4 : motionPin;

  // Status LEDs
  this->motionLEDPin = motionLEDPin < 0 ? 12 : motionLEDPin;

  // Timers auxiliar variables
  lastMeasure = 0;
  lastMotionTriggered = 0;
  lastBlinkCheck = 0;

  // Topics
  baseTopic = topicPrefix + "/motion";
  notificationTopic = baseTopic + "/notification";
  statusTopic = baseTopic + "/status";
}

void Motion::processMessage(PubSubClient& client, String topic, String messageTemp) {
  if(topic==baseTopic){
    changeStatus(client, messageTemp);
  }
}

void Motion::changeStatus(PubSubClient& client, String messageTemp)
{
  Serial.print("MOTION SENSOR STATUS CHANGE...");
  if(messageTemp == "1"){
    armMotion = true;
  } else if(messageTemp == "0"){
    armMotion=false;
    digitalWrite(motionLEDPin, LOW);
  }

  motionTriggered = false;
  this->reportStatus(client);
}

void Motion::reportStatus(PubSubClient& client)
{
  const char* statusLabel = armMotion ? "Armed" : "Not Armed";
  Serial.print("Motion Sensor ");
  Serial.println(statusLabel);
  client.publish(statusTopic.c_str(), statusLabel);
  client.publish(notificationTopic.c_str(), motionTriggered ? "MOTION DETECTED" : "NO MOTION");
}

void Motion::subscribe(PubSubClient& client) {
  client.subscribe(baseTopic.c_str());
  //client.subscribe(notificationTopic.c_str());
  //client.subscribe(statusTopic.c_str());
}

// Checks motion
// Do not publish anything to MQTT here, it breaks the whole connection
void Motion::detectsMovement() {
  if (armMotion) {
    if (!motionTriggered) {
      motionFirstTimeTriggered = true;
    }
    
    motionTriggered = true;
  }
}

Motion* signalHandler = NULL;

void memberFunctionWrapper()
{
  signalHandler->detectsMovement();
}

// The setup function sets your ESP GPIOs to Outputs, starts the serial communication at a baud rate of 115200
// Sets your mqtt broker and sets the callback function
// The callback function is what receives messages and actually controls the LEDs
void Motion::setup() {
  pinMode(motionLEDPin, OUTPUT);

  signalHandler = this;
  attachInterrupt(digitalPinToInterrupt(motionSensor), memberFunctionWrapper, RISING);
}

void Motion::loop(PubSubClient& client) {
  // Process motion detection
  long now = millis();
  if (armMotion && motionTriggered) {
    noInterrupts();
    if (motionFirstTimeTriggered) {
      Serial.println("MOTION DETECTED!!!");
      lastMotionTriggered = now;
      client.publish(notificationTopic.c_str(), "MOTION DETECTED");
      motionFirstTimeTriggered = false;
      digitalWrite(motionLEDPin, HIGH);
    } else if (now - lastMotionTriggered > 500) {
        motionTriggered = false;
        client.publish(notificationTopic.c_str(), "NO MOTION");
        digitalWrite(motionLEDPin, LOW);
    }
    interrupts();
  }
}

