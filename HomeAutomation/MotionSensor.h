#ifndef _MOTION_H_
#define _MOTION_H_

// Loading the required libraries
#include <PubSubClient.h>

class Motion {
  public:
    Motion(uint8_t motionPin, uint8_t motionLEDPin, String topicPrefix);

    void processMessage(PubSubClient& client, String topic, String messageTemp);
    void changeStatus(PubSubClient& client, String messageTemp);
    void reportStatus(PubSubClient& client);
    void subscribe(PubSubClient& client);
    void setup();
    void loop(PubSubClient& client);
    void detectsMovement();

    boolean armed() { return armMotion; };
    boolean triggered() { return motionTriggered; };

  private:
    // Control Variables
    boolean armMotion;
    boolean motionTriggered;
    boolean motionFirstTimeTriggered;

    // PIR Motion Sensor pin
    int motionSensor;

    // Status LEDs pin
    int motionLEDPin;

    // Timers auxiliar variables
    long lastMeasure;
    long lastMotionTriggered;
    long lastBlinkCheck;

    // Topic variables
    String baseTopic;
    String notificationTopic;
    String statusTopic;
};

#endif
