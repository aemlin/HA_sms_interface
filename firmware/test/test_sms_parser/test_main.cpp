#include <unity.h>
#include <Arduino.h>

// ---------------------------------------------------------------------------
// SMS parser utility under test
// Simulates parsing of AT+CMGR raw modem responses
// ---------------------------------------------------------------------------

struct SmsMessage {
    String sender;
    String timestamp;
    String body;
};

static bool parseAtCmgr(const String& raw, SmsMessage& out) {
    // Expected format:
    // +CMGR: "REC UNREAD","+32499000000",,"26/04/30,10:00:00+04"\r\nHello World
    int headerEnd = raw.indexOf('\n');
    if (headerEnd < 0) return false;

    String header = raw.substring(0, headerEnd);
    String body   = raw.substring(headerEnd + 1);
    body.trim();

    // Extract sender between second and third quote pair
    int q1 = header.indexOf('"', header.indexOf(',') + 1);
    int q2 = header.indexOf('"', q1 + 1);
    if (q1 < 0 || q2 < 0) return false;
    out.sender = header.substring(q1 + 1, q2);

    // Extract timestamp (last quoted field)
    int q3 = header.lastIndexOf('"');
    int q4 = header.lastIndexOf('"', q3 - 1);
    out.timestamp = (q4 >= 0 && q3 > q4) ? header.substring(q4 + 1, q3) : "";

    out.body = body;
    return out.sender.length() > 0 && out.body.length() > 0;
}

// ---------------------------------------------------------------------------
// Tests
// ---------------------------------------------------------------------------

void test_parse_valid_cmgr() {
    String raw = "+CMGR: \"REC UNREAD\",\"+32499000000\",,\"26/04/30,10:00:00+04\"\nHello World";
    SmsMessage msg;
    TEST_ASSERT_TRUE(parseAtCmgr(raw, msg));
    TEST_ASSERT_EQUAL_STRING("+32499000000", msg.sender.c_str());
    TEST_ASSERT_EQUAL_STRING("Hello World", msg.body.c_str());
}

void test_parse_empty_body_fails() {
    String raw = "+CMGR: \"REC UNREAD\",\"+32499000000\",,\"26/04/30,10:00:00+04\"\n";
    SmsMessage msg;
    TEST_ASSERT_FALSE(parseAtCmgr(raw, msg));
}

void test_parse_no_newline_fails() {
    String raw = "+CMGR: \"REC UNREAD\",\"+32499000000\",,\"26/04/30,10:00:00+04\"";
    SmsMessage msg;
    TEST_ASSERT_FALSE(parseAtCmgr(raw, msg));
}

void test_parse_multiline_body() {
    String raw = "+CMGR: \"REC UNREAD\",\"+32499000000\",,\"26/04/30,10:00:00+04\"\nLine1\nLine2";
    SmsMessage msg;
    TEST_ASSERT_TRUE(parseAtCmgr(raw, msg));
    // body captures first line only in this simplified parser
    TEST_ASSERT_TRUE(msg.body.startsWith("Line1"));
}

void test_parse_timestamp_extracted() {
    String raw = "+CMGR: \"REC UNREAD\",\"+32499000000\",,\"26/04/30,10:00:00+04\"\nHello";
    SmsMessage msg;
    TEST_ASSERT_TRUE(parseAtCmgr(raw, msg));
    TEST_ASSERT_EQUAL_STRING("26/04/30,10:00:00+04", msg.timestamp.c_str());
}

void test_parse_local_number() {
    // Number without international + prefix
    String raw = "+CMGR: \"REC UNREAD\",\"0499000000\",,\"26/04/30,10:00:00+04\"\nTest";
    SmsMessage msg;
    TEST_ASSERT_TRUE(parseAtCmgr(raw, msg));
    TEST_ASSERT_EQUAL_STRING("0499000000", msg.sender.c_str());
}

void test_parse_long_body() {
    String body(200, 'A'); // 200-character message
    String raw = "+CMGR: \"REC UNREAD\",\"+32499000000\",,\"26/04/30,10:00:00+04\"\n" + body;
    SmsMessage msg;
    TEST_ASSERT_TRUE(parseAtCmgr(raw, msg));
    TEST_ASSERT_EQUAL_INT(200, (int)msg.body.length());
}

void test_parse_body_with_special_chars() {
    String raw = "+CMGR: \"REC UNREAD\",\"+32499000000\",,\"26/04/30,10:00:00+04\"\npH: 6.8 / temp: 25.3C";
    SmsMessage msg;
    TEST_ASSERT_TRUE(parseAtCmgr(raw, msg));
    TEST_ASSERT_EQUAL_STRING("pH: 6.8 / temp: 25.3C", msg.body.c_str());
}

// ---------------------------------------------------------------------------

int main(int argc, char** argv) {
    UNITY_BEGIN();
    RUN_TEST(test_parse_valid_cmgr);
    RUN_TEST(test_parse_empty_body_fails);
    RUN_TEST(test_parse_no_newline_fails);
    RUN_TEST(test_parse_multiline_body);
    RUN_TEST(test_parse_timestamp_extracted);
    RUN_TEST(test_parse_local_number);
    RUN_TEST(test_parse_long_body);
    RUN_TEST(test_parse_body_with_special_chars);
    return UNITY_END();
}
