#ifndef _TEMPERATURE_H_
#define _TEMPERATURE_H_

// Loading the required libraries
#include <DHT.h>
#include <PubSubClient.h>

// Uncomment one of the lines bellow for whatever DHT sensor type you're using!
#define DHTTYPE DHT11 // DHT 11
//#define DHTTYPE DHT21 // DHT 21 (AM2301)
//#define DHTTYPE DHT22 // DHT 22 (AM2302), AM2321

// DHT Sensor
const uint16_t DHTTime = 30000;

class Temperature {
  public:
    Temperature(uint8_t dhtPin, uint8_t dhtType, uint16_t dhtTime, String topicPrefix);

    void setup();
    void loop(PubSubClient& client);
    
  private:
    DHT *dht;
  
    // Timers auxiliar variables
    long lastMeasureTime;
    uint16_t dhtTime;
    float lastTemperatureValue;
    float lastHumedityValue;

    // Topic variables
    String baseTopic;
    String temperatureTopic;
    String humedityTopic;

    void process(PubSubClient& client);
};

#endif
