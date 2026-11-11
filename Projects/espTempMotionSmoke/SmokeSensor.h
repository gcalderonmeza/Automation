#ifndef _Smoke_h
#define _Smoke_h

// Loading the required libraries
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

class Smoke {
  public:
    Smoke(int smokePin, int smokeLEDPin, int buzzerPin);

    void setBlinkTimeInMs(int blinkTimeInMs);
    void setSmokeThreshold(int smokeThreshold);

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
    int smokePin;
    int blinkTime;

    // Smoke Threshold
    int smokeThres;

    // Control Variables
    boolean smokeTriggered;
    boolean testMode;
    boolean blinkFlag;
    boolean blinkStatus;

    // Status LEDs
    int smokeLEDPin;

    // Buzzer
    int buzzerPin;

    // Timers auxiliar variables
    long lastMeasure;
    long lastSmokeCheck;
    long lastBlinkCheck;
    long testModeStart;
};

#endif

