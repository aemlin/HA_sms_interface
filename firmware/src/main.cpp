#include <Arduino.h>
#include "SmsGateway.h"

SmsGateway gateway;

void setup() {
    Serial.begin(115200);
    gateway.begin();
}

void loop() {
    gateway.loop();
}
