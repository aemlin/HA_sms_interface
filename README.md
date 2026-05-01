# LilyGO SMS Gateway

An SMS gateway built on a LilyGO T-SIM7000G (ESP32 + SIM7000G) that bridges SMS messages to Home Assistant via MQTT.

## Features

- Receive and send SMS via SIM7000G modem
- Publish incoming SMS to MQTT broker
- Subscribe to MQTT topics to send outgoing SMS
- Home Assistant automations for aquarium alerts and remote commands
- PlatformIO-based firmware

## Architecture

See [docs/architecture.md](docs/architecture.md) for a full overview.

## Hardware

- **Board**: LilyGO T-SIM7000G
- **Modem**: SIM7000G (LTE Cat-M / NB-IoT + GPRS)
- **MCU**: ESP32

## Quick Start

### Firmware

1. Install [PlatformIO](https://platformio.org/)
2. Copy `firmware/.env.example` to `firmware/.env` and fill in your credentials
3. Build and upload:

```bash
cd firmware
pio run --target upload
```

4. Monitor serial output:

```bash
pio device monitor
```

### Home Assistant

Copy the files from `homeassistant/` into your HA config directory and reload automations.

## MQTT Topics

| Topic | Direction | Description |
|---|---|---|
| `sms/incoming` | Publish | Incoming SMS payload (JSON) |
| `sms/send` | Subscribe | Send an SMS (JSON) |
| `sms/status` | Publish | Gateway status / heartbeat |

## License

MIT
