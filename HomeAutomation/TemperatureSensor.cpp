// Loading the required libraries
#include "TemperatureSensor.h"

Temperature::Temperature(uint8_t dhtPin, uint8_t dhtType, uint16_t dhtTime, String topicPrefix)
{
  // Copying the object to the heap
  this->dht = new DHT(dhtPin, dhtType);
  this->dhtTime = dhtTime;

  // Topics
  this->baseTopic = topicPrefix;
  this->temperatureTopic = this->baseTopic + F("/temperature");
  this->humedityTopic = this->baseTopic + F("/humedity");

  this->lastTemperatureValue = -10000;
  this->lastHumedityValue = -100;

  this->lastMeasureTime = millis();
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
  if ((now - this->lastMeasureTime) > this->dhtTime) {
    this->lastMeasureTime = now;
    this->process(client);
  }  
}

void Temperature::process(PubSubClient& client)
{
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  float h = this->dht->readHumidity();
  // Read temperature as Celsius (the default)
  float t = this->dht->readTemperature();
  // Read temperature as Fahrenheit (isFahrenheit = true)
  float f = this->dht->readTemperature(true);
  // Check if any reads failed and exit early (to try again).
  if (isnan(h) || isnan(t) || isnan(f)) {
  	if (isnan(h)) Serial.println(F("Failed to read humedity from DHT sensor!"));
  	if (isnan(t)) Serial.println(F("Failed to read temp (C) from DHT sensor!"));
  	if (isnan(f)) Serial.println(F("Failed to read temp (F) from DHT sensor!"));
	
  	return;
  } 
  
  // Computes temperature values in Celsius
  float hic = this->dht->computeHeatIndex(t, h, false);
  static char temperatureTemp[6];
  dtostrf(hic, 6, 2, temperatureTemp);

  if (this->lastTemperatureValue == 0 || abs((this->lastTemperatureValue - hic) / this->lastTemperatureValue) > 0.1)
  {
    // Publishes Temperature value if it changes more than 10% of the last value
    client.publish(this->temperatureTopic.c_str(), temperatureTemp);
  }

  // Uncomment to compute temperature values in Fahrenheit
  //float hif = dht.computeHeatIndex(f, h);
  //static char temperatureTempF[6];
  //dtostrf(hif, 6, 2, temperatureTempF);
  static char humidityTemp[6];
  dtostrf(h, 6, 2, humidityTemp);

  if (this->lastHumedityValue == 0 || abs((this->lastHumedityValue - h) / this->lastHumedityValue) > 0.1)
  {
    // Publishes Humidity value if it changes more than 10% of the last value
    client.publish(this->humedityTopic.c_str(), humidityTemp);
  }
  
  Serial.print(F("Humidity: "));
  Serial.print(h);
  Serial.print(F(" %\t Temperature: "));
  Serial.print(t);
  //Serial.print(" *F ");
  //Serial.print(f);
  Serial.print(F(" *C\t Heat index: "));
  Serial.print(hic);
  Serial.println(F(" *C "));
  //Serial.print(hif);
  //Serial.println(" *F");
}
