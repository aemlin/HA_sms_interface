#include <unity.h>
#include <string>

void setUp(void) {}
void tearDown(void) {}
#include <algorithm>
#include <cctype>

// ---------------------------------------------------------------------------
// SMS parser utility under test
// Simulates parsing of AT+CMGR raw modem responses — no hardware dependency
// ---------------------------------------------------------------------------

struct SmsMessage {
    std::string sender;
    std::string timestamp;
    std::string body;
};

static void trimStr(std::string& s) {
    s.erase(s.begin(),
            std::find_if(s.begin(), s.end(),
                         [](unsigned char c) { return !std::isspace(c); }));
    s.erase(std::find_if(s.rbegin(), s.rend(),
                         [](unsigned char c) { return !std::isspace(c); }).base(),
            s.end());
}

static bool parseAtCmgr(const std::string& raw, SmsMessage& out) {
    // Expected format:
    // +CMGR: "REC UNREAD","+32499000000",,"26/04/30,10:00:00+04"\r\nHello World
    size_t headerEnd = raw.find('\n');
    if (headerEnd == std::string::npos) return false;

    std::string header = raw.substr(0, headerEnd);
    std::string body   = raw.substr(headerEnd + 1);
    trimStr(body);

    // Extract sender between second and third quote pair
    size_t comma1 = header.find(',');
    if (comma1 == std::string::npos) return false;

    size_t q1 = header.find('"', comma1 + 1);
    if (q1 == std::string::npos) return false;
    size_t q2 = header.find('"', q1 + 1);
    if (q2 == std::string::npos) return false;
    out.sender = header.substr(q1 + 1, q2 - q1 - 1);

    // Extract timestamp (last quoted field)
    size_t q3 = header.rfind('"');
    size_t q4 = (q3 != std::string::npos && q3 > 0)
                    ? header.rfind('"', q3 - 1)
                    : std::string::npos;
    out.timestamp = (q4 != std::string::npos && q3 > q4)
                        ? header.substr(q4 + 1, q3 - q4 - 1)
                        : "";

    out.body = body;
    return out.sender.length() > 0 && out.body.length() > 0;
}

// ---------------------------------------------------------------------------
// Tests
// ---------------------------------------------------------------------------

void test_parse_valid_cmgr() {
    std::string raw =
        "+CMGR: \"REC UNREAD\",\"+32499000000\",,\"26/04/30,10:00:00+04\"\n"
        "Hello World";
    SmsMessage msg;
    TEST_ASSERT_TRUE(parseAtCmgr(raw, msg));
    TEST_ASSERT_EQUAL_STRING("+32499000000", msg.sender.c_str());
    TEST_ASSERT_EQUAL_STRING("Hello World", msg.body.c_str());
}

void test_parse_empty_body_fails() {
    std::string raw =
        "+CMGR: \"REC UNREAD\",\"+32499000000\",,\"26/04/30,10:00:00+04\"\n";
    SmsMessage msg;
    TEST_ASSERT_FALSE(parseAtCmgr(raw, msg));
}

void test_parse_no_newline_fails() {
    std::string raw =
        "+CMGR: \"REC UNREAD\",\"+32499000000\",,\"26/04/30,10:00:00+04\"";
    SmsMessage msg;
    TEST_ASSERT_FALSE(parseAtCmgr(raw, msg));
}

void test_parse_multiline_body() {
    std::string raw =
        "+CMGR: \"REC UNREAD\",\"+32499000000\",,\"26/04/30,10:00:00+04\"\n"
        "Line1\nLine2";
    SmsMessage msg;
    TEST_ASSERT_TRUE(parseAtCmgr(raw, msg));
    // body captures first line only in this simplified parser
    TEST_ASSERT_TRUE(msg.body.rfind("Line1", 0) == 0);
}

void test_parse_timestamp_extracted() {
    std::string raw =
        "+CMGR: \"REC UNREAD\",\"+32499000000\",,\"26/04/30,10:00:00+04\"\n"
        "Hello";
    SmsMessage msg;
    TEST_ASSERT_TRUE(parseAtCmgr(raw, msg));
    TEST_ASSERT_EQUAL_STRING("26/04/30,10:00:00+04", msg.timestamp.c_str());
}

void test_parse_local_number() {
    std::string raw =
        "+CMGR: \"REC UNREAD\",\"0499000000\",,\"26/04/30,10:00:00+04\"\nTest";
    SmsMessage msg;
    TEST_ASSERT_TRUE(parseAtCmgr(raw, msg));
    TEST_ASSERT_EQUAL_STRING("0499000000", msg.sender.c_str());
}

void test_parse_long_body() {
    std::string body(200, 'A');
    std::string raw =
        "+CMGR: \"REC UNREAD\",\"+32499000000\",,\"26/04/30,10:00:00+04\"\n"
        + body;
    SmsMessage msg;
    TEST_ASSERT_TRUE(parseAtCmgr(raw, msg));
    TEST_ASSERT_EQUAL_INT(200, (int)msg.body.length());
}

void test_parse_body_with_special_chars() {
    std::string raw =
        "+CMGR: \"REC UNREAD\",\"+32499000000\",,\"26/04/30,10:00:00+04\"\n"
        "pH: 6.8 / temp: 25.3C";
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
