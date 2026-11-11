// Loading the required libraries
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include "MotionSensor.h"
#include "TemperatureSensor.h"
#include "SwitchGroup.h"
#include "SimpleTimer.h"
#include "LightSensor.h"
#include "LightControlConstants.h"
#include "networkId.h"
#include "garage.h"

// Initializes the espClient
WiFiClient espClient;
PubSubClient client(espClient);

char* messages[] = { (char*)"010001010001010000000011", (char*)"010001010001010000001100", (char*)"010001010001010000110000", (char*)"010001010001010011000000", (char*)"010001010001011100000000" };
bool toggles[] = {true, false, true, true, false};

// Initializae the switch group
SwitchGroup switches(SwitchPin, 160, 7, topicPrefix);

// Initialize DHT sensor.
Temperature dht(DHTPin, DHTTYPE, DHTTime, topicPrefix);

// PIR Motion Sensor constants
const int motionSensor = 4;
const int motionLEDPin = 12;

Motion motion(motionSensor, motionLEDPin, topicPrefix);

// Light detection
LightSensor lightSensor(A0, 20, topicPrefix);
unsigned long lastLightMeasure = 0;

LightControlModeEnum lightControlMode = LightModeByLight; // ControlByLight, 1 = LightOn, 2 = LightOff

const int tvLight = 3;
const int stairsLight = 4;
const int frontLight = 1;

// Timers auxiliar variables
long now = millis();
long lastMeasure = 0;
long lastBlinkCheck = 0;

// Don't change the function below. This functions connects your ESP8266 to your router
void setup_wifi() {
  delay(10);

  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  Serial.println("");
  Serial.print("WiFi connected - ESP IP address: ");
  Serial.println(WiFi.localIP());
}

const String LightControlBaseTopicPrefix = topicPrefix + LightTopicPrefix;
const String LightControlStatusTopic = LightControlBaseTopicPrefix + LightStatusTopic;
const String LightControlModeTopic = LightControlBaseTopicPrefix + LightModeTopic;
const String LightControlThesholdTopic = LightControlBaseTopicPrefix + LightThesholdTopic;

void process_light_control_message(String topic, String messageTemp)
{
  if(topic==LightControlBaseTopicPrefix) { // "home/garage/esp1/light") {
    Serial.print("NEW MODE ARRIVED...");
    client.publish(LightControlStatusTopic.c_str(), LightStatusArmedMessage);
    if(messageTemp==TurnOnMessage) {
      lightControlMode = LightModeOn;
      client.publish(LightControlStatusTopic.c_str(), LightStatusArmedMessage);
      client.publish(LightControlModeTopic.c_str(), LightControlModeMessages[lightControlMode]);
      Serial.print("Light control status: LIGHT ON");
    } else if(messageTemp==TurnOffMessage) {
      lightControlMode = LightModeOff;
      client.publish(LightControlStatusTopic.c_str(), LightStatusArmedMessage);
      client.publish(LightControlModeTopic.c_str(), LightControlModeMessages[lightControlMode]);
      Serial.print("Light control status: LIGHT OFF");
    } else if(messageTemp==ControlByLightMessage) {
      lightControlMode = LightModeByLight;
      client.publish(LightControlStatusTopic.c_str(), LightStatusArmedMessage);
      client.publish(LightControlModeTopic.c_str(), LightControlModeMessages[lightControlMode]);
      Serial.print("Light control status: CONTROL BY LIGHT");
    }
  }

  if(topic==LightControlThesholdTopic) {
    Serial.print("NEW THRESHOLD ARRIVED...");
    Serial.print(messageTemp);

    lightSensor.SetThreshold(atoi(messageTemp.c_str()));
  }
}

// This functions is executed when some device publishes a message to a topic that your ESP8266 is subscribed to
// Change the function below to add logic to your program, so when a device publishes a message to a topic that
// your ESP8266 is subscribed you can actually do something
void callback(String topic, byte* message, unsigned int length) {
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  String messageTemp;
  for (unsigned int i = 0; i < length; i++) {
    Serial.print((char)message[i]);
    messageTemp += (char)message[i];
  }
  
  Serial.println();

  //switches.processMessage(client, topic, messageTemp, array with switches to toggle);
  motion.processMessage(client, topic, messageTemp);

  process_light_control_message(topic, messageTemp);
 
  Serial.println();
}

boolean reconnectedFlag = false;

// This functions reconnects your ESP8266 to your MQTT broker
// Change the function below if you want to subscribe to more topics with your ESP8266
void check_connection() {
    // Loop until we're reconnected
  if (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("GarageESP8266Client")) {
      Serial.println("connected");
      reconnectedFlag = true;

      // Publish "after reconnection states"
      motion.reportStatus(client);

      lightSensor.ReportThreshold(client);
      
      // Subscribe or resubscribe to topics
      // switches.subscribe(client);
     
      //client.subscribe("home/garage/esp1/workbench");

      motion.subscribe(client);

      client.subscribe("home/garage/esp1/light");
      client.subscribe("home/garage/esp1/light/threshold");
      delay(100);
    } else {
      Serial.print("Error: failed, rc=");
      Serial.println(client.state());
      // Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      // delay(5000);
    }
  }
  else
  {
    reconnectedFlag = false;
  }
}

// The setup function sets your ESP GPIOs to Outputs, starts the serial communication at a baud rate of 115200
// Sets your mqtt broker and sets the callback function
// The callback function is what receives messages and actually controls the LEDs
void setup() {
  motion.setup();
  dht.setup();
  
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  switches.setup(messages, messages, 5);
  lightSensor.setup();
}

// Basically the loop ensures that you ESP is connected to your broker and performs all the functions of the broker
void loop() {
  // Connect/reconnect to the broker
  check_connection();

  client.loop();

  now = millis();
  
  // Process temperature and humedity
  dht.loop(client);

  // Process motion detection
  motion.loop(client);

  if (lightSensor.IsArmed() && (now - lastLightMeasure > 500)) {
    lastLightMeasure = now;
    processLightMeasurement();
  }

  delay(100);
}

void processLightMeasurement()
{
  lightSensor.process(client);

  switch(lightControlMode)
  {
    case 0: 
      if (!lightSensor.IsArmed())
      {
        break;
      }
      
      // Control by light/motion
      if(!lightSensor.IsOverThreshold()) {
        if(motion.armed()) {
          if(motion.triggered() && !switches.getSwitchStatus(tvLight))
          {
            switches.turnOnSwitch(tvLight);
            delay(50);
            switches.turnOnSwitch(stairsLight);
            //delay(50);
            //switches.turnOnSwitch(frontLight);
            Serial.println("Some one moved, turn on lights!");
          } else {
            Serial.println("Keep light off, no one is moving");
          }
        } else if(!switches.getSwitchStatus(tvLight)) {
          switches.turnOnSwitch(tvLight);
          delay(50);
          switches.turnOnSwitch(stairsLight);
          delay(50);
          switches.turnOnSwitch(frontLight);
          Serial.println("It is too dark, turning light on!");
        }
      } else if(lightSensor.IsOverThreshold(1.1) && switches.getSwitchStatus(tvLight)) {
        Serial.println("It is not dark at all, turning light off!");
        switches.turnOffSwitch(tvLight);
        delay(50);
        switches.turnOffSwitch(stairsLight);
        delay(50);
        switches.turnOffSwitch(frontLight);
      }
      break;
    case 1:
      // Light on
      if(!switches.getSwitchStatus(tvLight))
      {
        switches.turnOnSwitch(tvLight);
        delay(50);
        switches.turnOnSwitch(stairsLight);
        delay(50);
        switches.turnOnSwitch(frontLight);
        Serial.println("Turning light on!");
      }
      break;
    case 2:
      // Light off
      if(switches.getSwitchStatus(tvLight))
      {
        switches.turnOffSwitch(tvLight);
        delay(50);
        switches.turnOffSwitch(stairsLight);
        delay(50);
        switches.turnOffSwitch(frontLight);
        Serial.println("Turning light off!");
      }
      break;
  }
}

