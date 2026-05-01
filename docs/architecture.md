# Architecture

## Overview

```
+------------------+        SMS         +-------------------+
|   Phone / SIM    | <--------------->  | LilyGO T-SIM7000G |
+------------------+                    |  (ESP32 + SIM7000G)|
                                        +---------+---------+
                                                  |
                                               MQTT (WiFi / LTE)
                                                  |
                                        +---------v---------+
                                        |   MQTT Broker     |
                                        |  (Mosquitto / HA) |
                                        +---------+---------+
                                                  |
                                        +---------v---------+
                                        |  Home Assistant   |
                                        |  automations      |
                                        +-------------------+
```

## Components

### Firmware

| Class | File | Responsibility |
|---|---|---|
| `SmsGateway` | `SmsGateway.h/.cpp` | Orchestration — ties modem and MQTT together |
| `ModemManager` | `ModemManager.h/.cpp` | AT command abstraction over SIM7000G |
| `MqttManager` | `MqttManager.h/.cpp` | MQTT connect / publish / subscribe lifecycle |

### MQTT Payload Schemas

**Incoming SMS** (`sms/incoming`):
```json
{
  "sender": "+32499000000",
  "timestamp": "2026-04-30T10:00:00Z",
  "message": "Hello World"
}
```

**Send SMS** (`sms/send`):
```json
{
  "recipient": "+32499000000",
  "message": "Alert: pH is too low!"
}
```

**Status** (`sms/status`):
```json
{
  "state": "online",
  "rssi": -72,
  "uptime_s": 3600
}
```

## Connectivity

The gateway connects to MQTT over WiFi by default. LTE fallback via SIM7000G can be enabled in `platformio.ini` build flags.
