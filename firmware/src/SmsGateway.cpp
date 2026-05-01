#include <ArduinoJson.h>
#include "SmsGateway.h"
#include "config.h"

void SmsGateway::begin() {
    // Register callbacks before begin() so no early event is missed
    _modem.onSmsReceived([this](const String& sender, const String& msg) {
        _onIncomingSms(sender, msg);
    });
    _mqtt.onSendCommand([this](const String& recipient, const String& msg) {
        _onMqttSendCommand(recipient, msg);
    });

    // WiFi + MQTT must be up before OTA and modem start
    _mqtt.begin();
    _ota.begin(OTA_HOSTNAME, OTA_PASSWORD);
    _modem.begin();
}

void SmsGateway::loop() {
    _ota.loop();   // highest priority — handle OTA before anything else
    _mqtt.loop();
    _modem.loop();
    _publishHeartbeat();
}

void SmsGateway::_onIncomingSms(const String& sender, const String& message) {
    Serial.printf("[Gateway] SMS from %s\n", sender.c_str());

    JsonDocument doc;
    doc["sender"]    = sender;
    doc["message"]   = message;
    doc["timestamp"] = _modem.getTimestamp();

    String payload;
    serializeJson(doc, payload);
    _mqtt.publish(MQTT_TOPIC_INCOMING, payload);
}

void SmsGateway::_onMqttSendCommand(const String& recipient, const String& message) {
    Serial.printf("[Gateway] Sending SMS to %s\n", recipient.c_str());
    _modem.sendSms(recipient, message);
}

void SmsGateway::_publishHeartbeat() {
    unsigned long now = millis();
    if (now - _lastHeartbeat < HEARTBEAT_MS) return;
    _lastHeartbeat = now;

    JsonDocument doc;
    doc["state"]    = "online";
    doc["rssi"]     = _modem.getRssi();
    doc["uptime_s"] = now / 1000UL;

    String payload;
    serializeJson(doc, payload);
    _mqtt.publish(MQTT_TOPIC_STATUS, payload);
}
