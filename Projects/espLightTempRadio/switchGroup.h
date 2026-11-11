#ifndef _SWITCHGROUP_H_
#define _SWITCHGROUP_H_

// Loading the required libraries
#include <RCSwitch.h>

const int SwitchPin = 16;
const int Protocol = 1;

class SwitchGroup {
  public:
    SwitchGroup(int switchPin, int pulseWidth, int protocol, String topicPrefix);

    void setup(char** onMessages, char** offMessages, int numSwitches);
    void subscribe(PubSubClient& client);
    void processMessage(PubSubClient& client, String topic, String messageTemp, bool switchesToToggle[]);
    void turnOnSwitch(int switchNumber);
    void turnOffSwitch(int switchNumber);
    bool getSwitchStatus(int switchNumber);
    
  private:
    RCSwitch mySwitch;

    //Setup the radio transmitter
    int switchPin;
    int pulseWidth;
    int protocol;

    // Topic variables
    String baseTopic;

    char** onMessages;
    char** offMessages;
    int numSwitches;
    bool* switchStatus;

    bool* switchesToToggle;

    void toggleSwitch(int switchNumber);
};

#endif
