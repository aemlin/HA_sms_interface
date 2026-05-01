#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include "MqttManager.h"
#include "config.h"

static WiFiClient   _wifiClient;
static PubSubClient _mqttClient(_wifiClient);

// PubSubClient requires a plain C callback; route it to the single instance.
static MqttManager* _instance = nullptr;

static void mqttCallback(char* topic, uint8_t* payload, unsigned int length) {
    if (_instance) _instance->_onMessage(topic, payload, length);
}

// LWT payload broadcast when the device disconnects ungracefully
static const char LWT_PAYLOAD[] = "{\"state\":\"offline\"}";

// ---------------------------------------------------------------------------

void MqttManager::begin() {
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    Serial.print(F("[WiFi] Connecting"));
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print('.');
    }
    Serial.printf("\n[WiFi] IP: %s\n", WiFi.localIP().toString().c_str());

    _mqttClient.setServer(MQTT_HOST, MQTT_PORT);
    _mqttClient.setCallback(mqttCallback);
    _mqttClient.setBufferSize(1024);
    _instance = this;

    _connect();
}

void MqttManager::loop() {
    if (!_mqttClient.connected()) {
        static unsigned long _lastAttempt = 0;
        unsigned long now = millis();
        if (now - _lastAttempt >= MQTT_RECONNECT_MS) {
            _lastAttempt = now;
            _connect();
        }
    }
    _mqttClient.loop();
}

void MqttManager::publish(const char* topic, const String& payload) {
    if (!_mqttClient.connected()) return;
    _mqttClient.publish(topic, payload.c_str());
}

// ---------------------------------------------------------------------------

void MqttManager::_connect() {
    Serial.print(F("[MQTT] Connecting..."));

    const char* user = strlen(MQTT_USER)     ? MQTT_USER     : nullptr;
    const char* pass = strlen(MQTT_PASSWORD) ? MQTT_PASSWORD : nullptr;

    // Last Will Testament: broker publishes "offline" status if we drop silently
    bool ok = _mqttClient.connect(
        MQTT_CLIENT_ID, user, pass,
        MQTT_TOPIC_STATUS, /*qos*/ 1, /*retain*/ true, LWT_PAYLOAD);

    if (ok) {
        _mqttClient.subscribe(MQTT_TOPIC_SEND);
        Serial.println(F(" OK"));
    } else {
        Serial.printf(" FAIL rc=%d\n", _mqttClient.state());
    }
    // Non-blocking: loop() retries after MQTT_RECONNECT_MS if not connected
}

void MqttManager::_onMessage(const char* topic, uint8_t* payload, unsigned int length) {
    if (strcmp(topic, MQTT_TOPIC_SEND) != 0 || !_sendCb) return;

    JsonDocument doc;
    if (deserializeJson(doc, payload, length) != DeserializationError::Ok) return;
    if (!doc["recipient"].is<const char*>() || !doc["message"].is<const char*>()) return;

    _sendCb(doc["recipient"].as<String>(), doc["message"].as<String>());
}
