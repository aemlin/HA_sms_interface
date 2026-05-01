#include <ArduinoOTA.h>
#include "OtaManager.h"

void OtaManager::begin(const char* hostname, const char* password) {
    ArduinoOTA.setHostname(hostname);
    ArduinoOTA.setPort(3232);

    if (password && strlen(password) > 0) {
        ArduinoOTA.setPassword(password);
    }

    ArduinoOTA.onStart([]() {
        // Flush serial before the update silences it
        String type = (ArduinoOTA.getCommand() == U_FLASH) ? "firmware" : "filesystem";
        Serial.printf("[OTA] Start — type: %s\n", type.c_str());
    });

    ArduinoOTA.onEnd([]() {
        Serial.println(F("[OTA] Done — rebooting"));
    });

    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
        static unsigned int lastPct = 0;
        unsigned int pct = progress * 100 / total;
        if (pct != lastPct && pct % 10 == 0) {
            Serial.printf("[OTA] %u%%\n", pct);
            lastPct = pct;
        }
    });

    ArduinoOTA.onError([](ota_error_t error) {
        const char* msg = "unknown";
        switch (error) {
            case OTA_AUTH_ERROR:    msg = "auth failed";    break;
            case OTA_BEGIN_ERROR:   msg = "begin failed";   break;
            case OTA_CONNECT_ERROR: msg = "connect failed"; break;
            case OTA_RECEIVE_ERROR: msg = "receive failed"; break;
            case OTA_END_ERROR:     msg = "end failed";     break;
        }
        Serial.printf("[OTA] Error: %s\n", msg);
    });

    ArduinoOTA.begin();
    Serial.printf("[OTA] Ready — hostname: %s  port: 3232\n", hostname);
}

void OtaManager::loop() {
    ArduinoOTA.handle();
}
