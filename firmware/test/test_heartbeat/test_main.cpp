#include <unity.h>
#include <ArduinoJson.h>

// ---------------------------------------------------------------------------
// Logic under test — mirrors SmsGateway._publishHeartbeat() internals
// ---------------------------------------------------------------------------

static bool isHeartbeatDue(unsigned long lastMs, unsigned long nowMs,
                            unsigned long intervalMs) {
    return (nowMs - lastMs) >= intervalMs;
}

static String buildStatusPayload(const char* state, int rssi,
                                 unsigned long uptimeS) {
    JsonDocument doc;
    doc["state"]    = state;
    doc["rssi"]     = rssi;
    doc["uptime_s"] = uptimeS;
    String out;
    serializeJson(doc, out);
    return out;
}

// ---------------------------------------------------------------------------
// Heartbeat timing tests
// ---------------------------------------------------------------------------

void test_heartbeat_not_due() {
    TEST_ASSERT_FALSE(isHeartbeatDue(1000, 5000, 60000UL));
}

void test_heartbeat_due() {
    TEST_ASSERT_TRUE(isHeartbeatDue(0, 60001, 60000UL));
}

void test_heartbeat_due_at_exact_boundary() {
    TEST_ASSERT_TRUE(isHeartbeatDue(0, 60000, 60000UL));
}

void test_heartbeat_not_due_just_before_boundary() {
    TEST_ASSERT_FALSE(isHeartbeatDue(0, 59999, 60000UL));
}

void test_heartbeat_millis_overflow() {
    // Unsigned subtraction wraps correctly even when millis() rolls over ~49 days.
    // last = UINT32_MAX - 30000, now = 30000  →  elapsed = 60001
    unsigned long last = 0xFFFFFFFFUL - 30000UL;
    unsigned long now  = 30001UL;
    TEST_ASSERT_TRUE(isHeartbeatDue(last, now, 60000UL));
}

void test_heartbeat_millis_overflow_not_due() {
    // elapsed = 30000 < 60000
    unsigned long last = 0xFFFFFFFFUL - 10000UL;
    unsigned long now  = 20001UL;
    TEST_ASSERT_FALSE(isHeartbeatDue(last, now, 60000UL));
}

// ---------------------------------------------------------------------------
// Status payload tests
// ---------------------------------------------------------------------------

void test_status_payload_online() {
    String payload = buildStatusPayload("online", -72, 3600);
    JsonDocument doc;
    TEST_ASSERT_EQUAL(DeserializationError::Ok, deserializeJson(doc, payload));
    TEST_ASSERT_EQUAL_STRING("online", doc["state"].as<const char*>());
    TEST_ASSERT_EQUAL_INT(-72, doc["rssi"].as<int>());
    TEST_ASSERT_EQUAL_UINT32(3600, doc["uptime_s"].as<unsigned long>());
}

void test_status_payload_offline() {
    String payload = buildStatusPayload("offline", 0, 0);
    JsonDocument doc;
    deserializeJson(doc, payload);
    TEST_ASSERT_EQUAL_STRING("offline", doc["state"].as<const char*>());
    TEST_ASSERT_EQUAL_INT(0, doc["rssi"].as<int>());
}

void test_status_payload_is_valid_json() {
    String payload = buildStatusPayload("online", -85, 120);
    JsonDocument doc;
    TEST_ASSERT_EQUAL(DeserializationError::Ok, deserializeJson(doc, payload));
}

void test_status_payload_uptime_large() {
    // Verify no overflow for ~13 days uptime (1 200 000 s)
    String payload = buildStatusPayload("online", -60, 1200000UL);
    JsonDocument doc;
    deserializeJson(doc, payload);
    TEST_ASSERT_EQUAL_UINT32(1200000UL, doc["uptime_s"].as<unsigned long>());
}

// ---------------------------------------------------------------------------

int main(int argc, char** argv) {
    UNITY_BEGIN();
    RUN_TEST(test_heartbeat_not_due);
    RUN_TEST(test_heartbeat_due);
    RUN_TEST(test_heartbeat_due_at_exact_boundary);
    RUN_TEST(test_heartbeat_not_due_just_before_boundary);
    RUN_TEST(test_heartbeat_millis_overflow);
    RUN_TEST(test_heartbeat_millis_overflow_not_due);
    RUN_TEST(test_status_payload_online);
    RUN_TEST(test_status_payload_offline);
    RUN_TEST(test_status_payload_is_valid_json);
    RUN_TEST(test_status_payload_uptime_large);
    return UNITY_END();
}
