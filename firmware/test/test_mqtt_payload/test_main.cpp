#include <unity.h>
#include <ArduinoJson.h>
#include <string>

void setUp(void) {}
void tearDown(void) {}

// ---------------------------------------------------------------------------
// Helpers under test (extracted logic, no hardware dependencies)
// ---------------------------------------------------------------------------

static bool parseSendCommand(const char* raw, std::string& recipient,
                             std::string& message) {
    JsonDocument doc;
    if (deserializeJson(doc, raw) != DeserializationError::Ok) return false;
    if (!doc["recipient"].is<const char*>() || !doc["message"].is<const char*>())
        return false;
    recipient = doc["recipient"].as<std::string>();
    message   = doc["message"].as<std::string>();
    return true;
}

static std::string buildIncomingPayload(const char* sender, const char* msg,
                                        const char* ts) {
    JsonDocument doc;
    doc["sender"]    = sender;
    doc["message"]   = msg;
    doc["timestamp"] = ts;
    std::string out;
    serializeJson(doc, out);
    return out;
}

// ---------------------------------------------------------------------------
// Tests
// ---------------------------------------------------------------------------

void test_parse_valid_send_command() {
    std::string recipient, message;
    bool ok = parseSendCommand(
        R"({"recipient":"+32499000000","message":"Hello"})",
        recipient, message);

    TEST_ASSERT_TRUE(ok);
    TEST_ASSERT_EQUAL_STRING("+32499000000", recipient.c_str());
    TEST_ASSERT_EQUAL_STRING("Hello", message.c_str());
}

void test_parse_missing_recipient() {
    std::string recipient, message;
    bool ok = parseSendCommand(R"({"message":"Hello"})", recipient, message);
    TEST_ASSERT_FALSE(ok);
}

void test_parse_invalid_json() {
    std::string recipient, message;
    bool ok = parseSendCommand("not json", recipient, message);
    TEST_ASSERT_FALSE(ok);
}

void test_build_incoming_payload_contains_sender() {
    std::string payload = buildIncomingPayload("+32499000000", "Test",
                                               "2026-04-30T10:00:00Z");
    TEST_ASSERT_TRUE(payload.find("+32499000000") != std::string::npos);
}

void test_build_incoming_payload_valid_json() {
    std::string payload = buildIncomingPayload("+32499000000", "Test",
                                               "2026-04-30T10:00:00Z");
    JsonDocument doc;
    TEST_ASSERT_TRUE(deserializeJson(doc, payload) == DeserializationError::Ok);
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
