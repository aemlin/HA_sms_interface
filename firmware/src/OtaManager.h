#pragma once

#include <Arduino.h>

class OtaManager {
public:
    void begin(const char* hostname, const char* password);
    void loop();
};
