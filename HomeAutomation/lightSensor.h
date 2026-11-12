#ifndef _LIGHT_SENSOR_H_
#define _LIGHT_SENSOR_H_

#include <PubSubClient.h>

class LightSensor {
  public:
    LightSensor(int lightSensorPin, int threshold, String topicPrefix);

    void setup();
    void loop(PubSubClient& client);

    int SetThreshold(int newThreshold);
    
    float GetLightValue();
    bool IsOverThreshold();
    bool IsOverThreshold(float gap);

    void ReportValue(PubSubClient& client);
    void ReportThreshold(PubSubClient& client);

    void process(PubSubClient& client);

    void Arm();
    void Disarm();
    bool IsArmed();
    
  private:
    bool isArmed;
  
    int lightPin;
    long lastLightMeasureTime;
    int lightThreshold;
  
    // Timers auxiliar variables
    long lightSensorTime;
    int lastLightValue;

    // Topic variables
    String baseTopic;
    String lightValueTopic;
    String lightThresholdTopic;
};

#endif
