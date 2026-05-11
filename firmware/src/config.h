#pragma once

// LilyGO T-SIM7000G hardware pins
#ifndef MODEM_TX_PIN
#define MODEM_TX_PIN     27
#endif
#ifndef MODEM_RX_PIN
#define MODEM_RX_PIN     26
#endif
#ifndef MODEM_PWRKEY_PIN
#define MODEM_PWRKEY_PIN  4
#endif
#ifndef MODEM_DTR_PIN
#define MODEM_DTR_PIN    25
#endif
#ifndef MODEM_STATUS_PIN
#define MODEM_STATUS_PIN 32
#endif

#ifndef MODEM_BAUD
#define MODEM_BAUD 9600
#endif

// WiFi (injected by scripts/load_env.py via build_flags)
#ifndef WIFI_SSID
#define WIFI_SSID ""
#endif
#ifndef WIFI_PASSWORD
#define WIFI_PASSWORD ""
#endif

// MQTT broker
#ifndef MQTT_HOST
#define MQTT_HOST "127.0.0.1"
#endif
#ifndef MQTT_PORT
#define MQTT_PORT 1883
#endif
#ifndef MQTT_USER
#define MQTT_USER ""
#endif
#ifndef MQTT_PASSWORD
#define MQTT_PASSWORD ""
#endif
#ifndef MQTT_CLIENT_ID
#define MQTT_CLIENT_ID "lilygo-sms-gateway"
#endif

// MQTT topics
#ifndef MQTT_TOPIC_INCOMING
#define MQTT_TOPIC_INCOMING "sms/incoming"
#endif
#ifndef MQTT_TOPIC_SEND
#define MQTT_TOPIC_SEND "sms/send"
#endif
#ifndef MQTT_TOPIC_STATUS
#define MQTT_TOPIC_STATUS "sms/status"
#endif

// SIM / APN
#ifndef SIM_APN
#define SIM_APN ""
#endif
#ifndef SIM_PIN
#define SIM_PIN ""
#endif

// Default SMS recipient — used when sms/send MQTT payload has no "recipient" field
#ifndef DEFAULT_RECIPIENT
#define DEFAULT_RECIPIENT ""
#endif

// Heartbeat interval in seconds (default 60 s)
#ifndef HEARTBEAT_INTERVAL
#define HEARTBEAT_INTERVAL 60
#endif
#define HEARTBEAT_MS ((unsigned long)(HEARTBEAT_INTERVAL) * 1000UL)

// How often to attempt MQTT reconnect (ms)
#define MQTT_RECONNECT_MS 5000UL

// OTA (Over-The-Air update via WiFi)
#ifndef OTA_PASSWORD
#define OTA_PASSWORD ""          // empty = no auth (not recommended)
#endif
#ifndef OTA_HOSTNAME
#define OTA_HOSTNAME MQTT_CLIENT_ID
#endif
