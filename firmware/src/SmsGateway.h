#pragma once

#include "ModemManager.h"
#include "MqttManager.h"
#include "OtaManager.h"

class SmsGateway {
public:
    void begin();
    void loop();

private:
    ModemManager _modem;
    MqttManager  _mqtt;
    OtaManager   _ota;

    unsigned long _lastHeartbeat = 0;

    void _onIncomingSms(const String& sender, const String& message);
    void _onMqttSendCommand(const String& recipient, const String& message);
    void _publishHeartbeat();
};
