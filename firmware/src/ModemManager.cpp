#define TINY_GSM_MODEM_SIM7000
#include <TinyGsmClient.h>
#include "ModemManager.h"
#include "config.h"

static HardwareSerial SerialAT(1);
static TinyGsm        _gsm(SerialAT);

// ---------------------------------------------------------------------------
// Internal helpers
// ---------------------------------------------------------------------------

static void powerOnModem() {
    pinMode(MODEM_PWRKEY_PIN, OUTPUT);
    digitalWrite(MODEM_PWRKEY_PIN, LOW);
    delay(100);
    digitalWrite(MODEM_PWRKEY_PIN, HIGH);
    delay(1000);
    digitalWrite(MODEM_PWRKEY_PIN, LOW);
    delay(3000);
}

// Read raw bytes from SerialAT until "OK" / "ERROR" or timeout.
// Used only for SMS-specific commands that TinyGSM doesn't abstract.
static String readAtResponse(unsigned long timeoutMs = 2000) {
    String resp;
    resp.reserve(256);
    unsigned long start = millis();
    while (millis() - start < timeoutMs) {
        while (SerialAT.available()) {
            resp += static_cast<char>(SerialAT.read());
        }
        if (resp.indexOf(F("OK")) >= 0 || resp.indexOf(F("ERROR")) >= 0) break;
    }
    return resp;
}

// ---------------------------------------------------------------------------
// Public API
// ---------------------------------------------------------------------------

void ModemManager::begin() {
    SerialAT.begin(MODEM_BAUD, SERIAL_8N1, MODEM_RX_PIN, MODEM_TX_PIN);
    powerOnModem();
    _gsm.restart();

    if (strlen(SIM_PIN) > 0) _gsm.simUnlock(SIM_PIN);

    Serial.print(F("[Modem] Waiting for network"));
    while (!_gsm.waitForNetwork(30000UL)) {
        Serial.print('.');
        delay(1000);
    }
    Serial.println(F(" OK"));

    // SMS text mode
    _gsm.sendAT(GF("+CMGF=1"));
    _gsm.waitResponse();

    // Route new-SMS notifications as unsolicited +CMTI lines
    _gsm.sendAT(GF("+CNMI=1,1,0,0,0"));
    _gsm.waitResponse();

    Serial.printf("[Modem] Signal: %d dBm\n", getRssi());
}

void ModemManager::loop() {
    _checkIncoming();
}

void ModemManager::sendSms(const String& recipient, const String& message) {
    Serial.printf("[Modem] SMS → %s\n", recipient.c_str());
    _gsm.sendSMS(recipient, message);
}

String ModemManager::getTimestamp() {
    String ts = _gsm.getGSMDateTime(DATE_FULL);
    return ts.length() ? ts : String(F("1970-01-01T00:00:00Z"));
}

int ModemManager::getRssi() {
    int csq = _gsm.getSignalQuality();
    if (csq == 0 || csq == 99) return 0;
    return -113 + 2 * csq; // CSQ → dBm
}

// ---------------------------------------------------------------------------
// Private
// ---------------------------------------------------------------------------

void ModemManager::_checkIncoming() {
    // Accumulate bytes from the modem UART; process complete lines
    while (SerialAT.available()) {
        char c = static_cast<char>(SerialAT.read());
        if (c == '\n') {
            _atBuf.trim();
            // Unsolicited new-SMS notification: +CMTI: "SM",<index>
            if (_atBuf.startsWith(F("+CMTI:"))) {
                int comma = _atBuf.indexOf(',');
                if (comma >= 0) {
                    int idx = _atBuf.substring(comma + 1).toInt();
                    String sender, message;
                    if (_parseSmsIndex(idx, sender, message) && _smsCallback) {
                        _smsCallback(sender, message);
                    }
                    // Delete from modem storage after processing
                    SerialAT.print(F("AT+CMGD="));
                    SerialAT.print(idx);
                    SerialAT.print(F("\r\n"));
                    readAtResponse(1000);
                }
            }
            _atBuf = "";
        } else {
            _atBuf += c;
        }
    }
}

bool ModemManager::_parseSmsIndex(int index, String& sender, String& message) {
    SerialAT.print(F("AT+CMGR="));
    SerialAT.print(index);
    SerialAT.print(F("\r\n"));
    String resp = readAtResponse(3000);

    // Response: +CMGR: "REC UNREAD","+32499000000",,"26/04/30,10:00:00+04"\r\n<body>\r\nOK
    int headerStart = resp.indexOf(F("+CMGR:"));
    if (headerStart < 0) return false;

    int headerEnd = resp.indexOf('\n', headerStart);
    if (headerEnd < 0) return false;

    String header = resp.substring(headerStart, headerEnd);
    header.trim();

    // Sender sits between the 2nd pair of double-quotes (after the status field)
    int q1 = header.indexOf('"', header.indexOf(',') + 1);
    int q2 = header.indexOf('"', q1 + 1);
    if (q1 < 0 || q2 <= q1) return false;
    sender = header.substring(q1 + 1, q2);

    int bodyStart = headerEnd + 1;
    int bodyEnd   = resp.indexOf('\n', bodyStart);
    message = resp.substring(bodyStart, bodyEnd < 0 ? resp.length() : bodyEnd);
    message.trim();

    return sender.length() > 0 && message.length() > 0;
}
