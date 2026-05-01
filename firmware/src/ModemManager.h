#pragma once

#include <Arduino.h>
#include <functional>

using SmsCallback = std::function<void(const String& sender, const String& message)>;

class ModemManager {
public:
    void begin();
    void loop();

    void sendSms(const String& recipient, const String& message);
    String getTimestamp();
    int    getRssi();

    void onSmsReceived(SmsCallback cb) { _smsCallback = cb; }

private:
    SmsCallback _smsCallback;
    String      _atBuf;   // accumulates bytes between newlines from SerialAT

    void _checkIncoming();
    bool _parseSmsIndex(int index, String& sender, String& message);
};
