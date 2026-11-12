#ifndef _SWITCHGROUP_H_
#define _SWITCHGROUP_H_

// Loading the required libraries
#include <RCSwitch.h>
#include <PubSubClient.h>

class SwitchGroup {
  public:
    SwitchGroup(uint8_t switchPin, int pulseWidth, uint8_t protocol, String topicPrefix);

    void setup(char** onMessages, char** offMessages, uint8_t numSwitches);
    void subscribe(PubSubClient& client);
    void processMessage(PubSubClient& client, String topic, String messageTemp, bool switchesToToggle[]);
    void turnOnSwitch(uint8_t switchNumber);
    void turnOffSwitch(uint8_t switchNumber);
    bool getSwitchStatus(uint8_t switchNumber);
    
  private:
    RCSwitch mySwitch;

    //Setup the radio transmitter
    uint8_t switchPin;
    int pulseWidth;
    uint8_t protocol;

    // Topic variables
    String baseTopic;

    char** onMessages;
    char** offMessages;
    int numSwitches;
    bool* switchStatus;

    bool* switchesToToggle;

    void toggleSwitch(uint8_t switchNumber);
};

#endif
