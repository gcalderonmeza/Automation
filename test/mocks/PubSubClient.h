#pragma once
#include "Arduino.h"
#include <vector>
#include <string>

// Minimal PubSubClient mock that records publish/subscribe calls.
class PubSubClient {
public:
    struct Call { std::string topic; std::string payload; };

    std::vector<Call>        published;
    std::vector<std::string> subscribed;

    bool publish(const char* topic, const char* payload) {
        published.push_back({topic, payload});
        return true;
    }

    bool subscribe(const char* topic) {
        subscribed.push_back(topic);
        return true;
    }

    bool connected() { return true; }
    bool loop()      { return true; }

    void reset() {
        published.clear();
        subscribed.clear();
    }

    bool wasPublished(const char* topic, const char* payload) const {
        for (const auto& c : published)
            if (c.topic == topic && c.payload == payload) return true;
        return false;
    }
};
