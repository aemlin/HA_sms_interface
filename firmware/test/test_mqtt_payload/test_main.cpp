#include <unity.h>
#include <ArduinoJson.h>

// ---------------------------------------------------------------------------
// Helpers under test (extracted logic, no hardware dependencies)
// ---------------------------------------------------------------------------

static bool parseSendCommand(const char* raw, String& recipient, String& message) {
    JsonDocument doc;
    if (deserializeJson(doc, raw) != DeserializationError::Ok) return false;
    if (!doc["recipient"].is<const char*>() || !doc["message"].is<const char*>()) return false;
    recipient = doc["recipient"].as<String>();
    message   = doc["message"].as<String>();
    return true;
}

static String buildIncomingPayload(const char* sender, const char* msg, const char* ts) {
    JsonDocument doc;
    doc["sender"]    = sender;
    doc["message"]   = msg;
    doc["timestamp"] = ts;
    String out;
    serializeJson(doc, out);
    return out;
}

// ---------------------------------------------------------------------------
// Tests
// ---------------------------------------------------------------------------

void test_parse_valid_send_command() {
    String recipient, message;
    bool ok = parseSendCommand(
        R"({"recipient":"+32499000000","message":"Hello"})",
        recipient, message);

    TEST_ASSERT_TRUE(ok);
    TEST_ASSERT_EQUAL_STRING("+32499000000", recipient.c_str());
    TEST_ASSERT_EQUAL_STRING("Hello", message.c_str());
}

void test_parse_missing_recipient() {
    String recipient, message;
    bool ok = parseSendCommand(R"({"message":"Hello"})", recipient, message);
    TEST_ASSERT_FALSE(ok);
}

void test_parse_invalid_json() {
    String recipient, message;
    bool ok = parseSendCommand("not json", recipient, message);
    TEST_ASSERT_FALSE(ok);
}

void test_build_incoming_payload_contains_sender() {
    String payload = buildIncomingPayload("+32499000000", "Test", "2026-04-30T10:00:00Z");
    TEST_ASSERT_TRUE(payload.indexOf("+32499000000") >= 0);
}

void test_build_incoming_payload_valid_json() {
    String payload = buildIncomingPayload("+32499000000", "Test", "2026-04-30T10:00:00Z");
    JsonDocument doc;
    TEST_ASSERT_EQUAL(DeserializationError::Ok, deserializeJson(doc, payload));
}

// ---------------------------------------------------------------------------

int main(int argc, char** argv) {
    UNITY_BEGIN();
    RUN_TEST(test_parse_valid_send_command);
    RUN_TEST(test_parse_missing_recipient);
    RUN_TEST(test_parse_invalid_json);
    RUN_TEST(test_build_incoming_payload_contains_sender);
    RUN_TEST(test_build_incoming_payload_valid_json);
    return UNITY_END();
}
