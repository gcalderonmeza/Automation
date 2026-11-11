// Loading the required libraries
#include <PubSubClient.h>
#include "TemperatureSensor.h"

Temperature::Temperature(int dhtPin, int dhtType, int dhtTime, String topicPrefix)
{
  this->dht = new DHT(dhtPin < 0 ? DHTPin : dhtPin, dhtType < 0 ? DHTTYPE : dhtType);
  this->dhtTime = dhtTime < 0 ? DHTTime : dhtTime;

  // Topics
  baseTopic = topicPrefix;
  temperatureTopic = baseTopic + "/temperature";
  humedityTopic = baseTopic + "/humedity";

  lastTemperatureValue = -10000;
  lastHumedityValue = -100;
}

/*
void Motion::reportStatus(PubSubClient& client)
{
  const char* statusLabel = armMotion ? "Armed" : "Not Armed";
  Serial.print("Motion Sensor ");
  Serial.print(statusLabel);
  client.publish(statusTopic.c_str(), statusLabel);
  client.publish(notificationTopic.c_str(), motionTriggered ? "MOTION DETECTED" : "NO MOTION");
}
*/
// The setup function sets your ESP GPIOs to Outputs, starts the serial communication at a baud rate of 115200
// Sets your mqtt broker and sets the callback function
// The callback function is what receives messages and actually controls the LEDs
void Temperature::setup() {
  this->dht->begin();
}

void Temperature::loop(PubSubClient& client) {
  // Process temperature and humedity
  long now = millis();
  if (now - lastMeasureTime > dhtTime) {
    lastMeasureTime = now;
    process(client);
  }  
}

void Temperature::process(PubSubClient& client)
{
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  float h = dht->readHumidity();
  // Read temperature as Celsius (the default)
  float t = dht->readTemperature();
  // Read temperature as Fahrenheit (isFahrenheit = true)
  // float f = dht.readTemperature(true);
  // Check if any reads failed and exit early (to try again).
  if (isnan(h) || isnan(t)) { //|| isnan(f)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }
  
  // Computes temperature values in Celsius
  float hic = dht->computeHeatIndex(t, h, false);
  static char temperatureTemp[6];
  dtostrf(hic, 6, 2, temperatureTemp);

  if (lastTemperatureValue == 0 || abs((lastTemperatureValue - hic) / lastTemperatureValue) > 0.1)
  {
    // Publishes Temperature value if it changes more than 10% of the last value
    client.publish(temperatureTopic.c_str(), temperatureTemp);
  }

  // Uncomment to compute temperature values in Fahrenheit
  //float hif = dht.computeHeatIndex(f, h);
  //static char temperatureTempF[6];
  //dtostrf(hif, 6, 2, temperatureTempF);
  static char humidityTemp[6];
  dtostrf(h, 6, 2, humidityTemp);

  if (lastHumedityValue == 0 || abs((lastHumedityValue - h) / lastHumedityValue) > 0.1)
  {
    // Publishes Humidity value if it changes more than 10% of the last value
    client.publish(humedityTopic.c_str(), humidityTemp);
  }
  
  Serial.print("Humidity: ");
  Serial.print(h);
  Serial.print(" %\t Temperature: ");
  Serial.print(t);
  //Serial.print(" *F ");
  //Serial.print(f);
  Serial.print(" *C\t Heat index: ");
  Serial.print(hic);
  Serial.println(" *C ");
  //Serial.print(hif);
  //Serial.println(" *F");
}

