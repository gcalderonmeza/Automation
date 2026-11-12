// Loading the required libraries
#if defined(ESP8266)
  #include <ESP8266WiFi.h>
#elif defined(ARDUINO_XIAO_ESP32S3)
  #include <WiFi.h>
#else
  #error "Unsupported board, add the Wifi library for your board"
#endif

#include "MotionSensor.h"
#include "TemperatureSensor.h"
#include "LightSensor.h"
#include "SwitchGroup.h"
#include "SimpleTimer.h"
#include "LightControlConstants.h"
#include "SmokeSensor.h"
// SETUP REQUIRED: Copy networkId.h.example to networkId.h and fill in your
// WiFi SSID, password, and MQTT broker address before compiling.
#include "NetworkId.h"
#include "AlesRoom.h"

#if defined(ESP8266)
  #define TEMPERATURE_SENSOR_PIN D1  // GPIO5
  #define MOTION_SENSOR_PIN      D2  // GPIO4
  #define MOTION_LED_PIN         D6  // GPIO12
  #define SMOKE_SENSOR_PIN       A0
  #define SMOKE_LED_PIN          D7  // GPIO13
  #define SMOKE_BUZZER_PIN       D5  // GPIO14
  #define SWITCH_GROUP_PIN       D0  // GPIO16
#elif defined(ARDUINO_XIAO_ESP32S3)
  #define TEMPERATURE_SENSOR_PIN D5  // GPIO6 aka A5
  #define MOTION_SENSOR_PIN      D4  // GPIO5 aka A4
  #define MOTION_LED_PIN         D9  // GPIO8
  #define SMOKE_SENSOR_PIN       A1  // GPIO2 aka D1
  #define SMOKE_LED_PIN          D10 // GPIO9
  #define SMOKE_BUZZER_PIN       D8  // GPIO7
  #define SWITCH_GROUP_PIN       D0  // GPIO1 aka A0
#else
  #error "Unsupported architecture — add pin definitions for your board"
#endif

// Initializes the espClient
WiFiClient espClient;
PubSubClient client(espClient);

// Temperature and humedity sensor: sensor read pin, sensor type, sensor read_time
Temperature dht(TEMPERATURE_SENSOR_PIN, DHTTYPE, DHTTime, topicPrefix);

// Motion sensor: sensor read pin, LED pin
Motion motion(MOTION_SENSOR_PIN, MOTION_LED_PIN, topicPrefix);

// Smoke Sensor
const int blinkTime = 500;
const int smokeThres = 70;
boolean smokeTriggered = false;
boolean testMode = false;
boolean blinkFlag = false;
boolean blinkStatus = true;

// Timers auxiliar variables
//long lastMeasure = 0;
long lastBlinkCheck = 0;

long lastSmokeCheck = 0;
long testModeStart = 0;

// Don't change the function below. This functions connects your ESP8266 to your router
void setup_wifi() {
  delay(10);

  // wifi_station_set_hostname("AlesRoom");

  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print(F("Connecting to "));
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  WiFi.hostname(F("ESP-Smoke"));
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(F("."));
  }
  
  Serial.println(F(""));
  Serial.print(F("WiFi connected - ESP IP address: "));
  Serial.print(WiFi.localIP());
  Serial.print(F(" - Host name: "));
  #if defined(ESP8266)
    Serial.println(WiFi.hostname());
  #elif defined(ARDUINO_XIAO_ESP32S3)
    Serial.println(WiFi.getHostname());
  #endif
}

// This functions is executed when some device publishes a message to a topic that your ESP8266 is subscribed to
// Change the function below to add logic to your program, so when a device publishes a message to a topic that
// your ESP8266 is subscribed you can actually do something
void callback(String topic, byte* message, unsigned int length) {
  Serial.print(F("Message arrived on topic: "));
  Serial.print(topic);
  Serial.print(F(". Message: "));
  String messageTemp;
  for (unsigned int i = 0; i < length; i++) {
    Serial.print((char)message[i]);
    messageTemp += (char)message[i];
  }
  
  Serial.println();

  motion.processMessage(client, topic, messageTemp);

  // Feel free to add more if statements to control more GPIOs with MQTT
  if(topic=="home/alesroom/esp1/smoke"){
    Serial.print(F("SMOKE SENSOR STATUS CHANGE... "));
    if (messageTemp=="Test/Reset") {
      if (smokeTriggered) {
        // Reset
        smokeTriggered = false;
        testMode = false;
        blinkFlag = false;
        blinkStatus = false;
        client.publish("home/alesroom/esp1/smoke/notification", "NO SMOKE");
        Serial.println(F("NO SMOKE"));
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
        client.publish("home/alesroom/esp1/smoke/notification", "IN TEST");
        Serial.println(F("IN TEST"));
      }

      blink_smoke(blinkStatus);
    }
  }
  
  Serial.println();
}

boolean reconnectedFlag = false;

// This functions reconnects your ESP8266 to your MQTT broker
// Change the function below if you want to subscribe to more topics with your ESP8266
void check_connection() {
    // Loop until we're reconnected
  if (!client.connected()) {
    Serial.print(F("Attempting MQTT connection..."));
    // Attempt to connect
    if (client.connect("AlesroomESP8266Client")) {
      Serial.println(F("connected"));
      reconnectedFlag = true;

      // Publish "after reconnection states"
      motion.reportStatus(client);
      
      // Subscribe or resubscribe to topics
      client.subscribe("home/alesroom/esp1/smoke");

      motion.subscribe(client);
      delay(100);
    } else {
      Serial.print(F("Error: failed, rc="));
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
  
  pinMode(SMOKE_LED_PIN, OUTPUT);
  pinMode(SMOKE_BUZZER_PIN, OUTPUT);

  pinMode(SMOKE_SENSOR_PIN, INPUT);
  
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  //mySwitch.enableTransmit(16);
}

// Basically the loop ensures that you ESP is connected to your broker and performs all the functions of the broker
void loop() {
  // Connect/reconnect to the broker
  check_connection();
  
  client.loop();

  // Process temperature and humedity
  dht.loop(client);

  // Process motion detection
  motion.loop(client);

  // Process smoke detection
  check_smoke();

  delay(100);
}

void report_smoke_status(boolean smokeTriggered)
{
  if (smokeTriggered) {
    client.publish("home/alesroom/esp1/smoke/notification", "SMOKE DETECTED!!!");
    Serial.println(F("SMOKE DETECTED!!!"));
  }
  else
  {
    // Not in test mode
    client.publish("home/alesroom/esp1/smoke/notification", "NO SMOKE");
    Serial.println(F("NO SMOKE"));
  }
}

void blink_smoke(boolean blinkStatus)
{
  if (blinkStatus) {
    digitalWrite(SMOKE_LED_PIN, HIGH);
    tone(SMOKE_BUZZER_PIN, 3400);
  }
  else
  {
    digitalWrite(SMOKE_LED_PIN, LOW);
    noTone(SMOKE_BUZZER_PIN);
  }
}

void check_smoke()
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
      blink_smoke(blinkStatus);
      testMode = false;
    }
    else if (now - lastBlinkCheck > blinkTime)
    {
      // Blink every second
      blinkStatus = !blinkStatus;
      blink_smoke(blinkStatus);
      lastBlinkCheck = now;
    }
  }
  else if (now - lastSmokeCheck > 2000) {
    // Read smoke after 2 secs
    lastSmokeCheck = now;
    int smokeValue = analogRead(SMOKE_SENSOR_PIN);
    Serial.print(F("Smoke value: "));
    Serial.println(smokeValue);

    smokeTriggered = smokeValue > smokeThres;
    if (smokeTriggered) {
      // Start blinking if not already doing it, if blinking already flip if the timer has been reached
      if (blinkFlag)
      {
        if (now - lastBlinkCheck > blinkTime)
        {
          blinkStatus = !blinkStatus;
          blink_smoke(blinkStatus);
          lastBlinkCheck = now;
        }
      }
      else
      {
        blinkFlag = true;
        blinkStatus = true;
        blink_smoke(blinkStatus);
        lastBlinkCheck = now;
      }
    }
    else
    {
      blinkFlag = false;
      blinkStatus = false;
      blink_smoke(blinkStatus);
    }

    report_smoke_status(smokeTriggered);
  }
}
