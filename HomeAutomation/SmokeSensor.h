#ifndef _Smoke_h
#define _Smoke_h

// Loading the required libraries
#include <PubSubClient.h>

class Smoke {
  public:
    Smoke(uint8_t smokePin, uint8_t smokeLEDPin, uint8_t buzzerPin);

    void setBlinkTimeInMs(uint16_t blinkTimeInMs);
    void setSmokeThreshold(uint8_t smokeThreshold);

    void startTestMode();
    void processMessage(PubSubClient& client, String topic, String messageTemp);
    void reportStatus(PubSubClient *client, boolean smokeTriggered);
    void blink(boolean blinkStatus);
    void subscribe(PubSubClient &client);
    void setup();
    void check(PubSubClient* client);
    void loop(PubSubClient* client);

  private:
    // Smoke Sensor
    uint8_t smokePin;
    uint16_t blinkTime;

    // Smoke Threshold
    uint8_t smokeThres;

    // Control Variables
    boolean smokeTriggered;
    boolean testMode;
    boolean blinkFlag;
    boolean blinkStatus;

    // Status LEDs
    uint8_t smokeLEDPin;

    // Buzzer
    uint8_t buzzerPin;

    // Timers auxiliar variables
    long lastMeasure;
    long lastSmokeCheck;
    long lastBlinkCheck;
    long testModeStart;
};

#endif

