#pragma once

#include <Arduino.h>
#include <functional>

using SendCommandCallback = std::function<void(const String& recipient, const String& message)>;

class MqttManager {
public:
    void begin();
    void loop();

    void publish(const char* topic, const String& payload);
    void onSendCommand(SendCommandCallback cb) { _sendCb = cb; }

private:
    SendCommandCallback _sendCb;

    void _connect();
    void _onMessage(const char* topic, uint8_t* payload, unsigned int length);

    friend void mqttCallback(char* topic, uint8_t* payload, unsigned int length);
};
