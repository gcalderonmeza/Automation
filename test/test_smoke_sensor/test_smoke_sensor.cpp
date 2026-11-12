#include "Arduino.h"

// Mock state — defined once per test binary (extern in Arduino.h).
uint32_t  _mock_millis   = 0;
int       _mock_analog[256] = {};
MockSerial Serial;

#include <unity.h>
#include "../../HomeAutomation/SmokeSensor.h"

static PubSubClient client;
static const int SMOKE_PIN = 0;

void setUp() {
    client.reset();
    _mock_millis = 0;
    memset(_mock_analog, 0, sizeof(_mock_analog));
}

void tearDown() {}

// ── Topic construction ────────────────────────────────────────────────────

void test_subscribe_uses_base_topic() {
    Smoke smoke(SMOKE_PIN, 1, 2, "home/alesroom/esp1");
    smoke.subscribe(client);
    TEST_ASSERT_EQUAL_STRING("home/alesroom/esp1/smoke",
                             client.subscribed[0].c_str());
}

void test_report_status_uses_notification_topic() {
    Smoke smoke(SMOKE_PIN, 1, 2, "home/alesroom/esp1");
    smoke.reportStatus(client);
    TEST_ASSERT_EQUAL_STRING("home/alesroom/esp1/smoke/notification",
                             client.published[0].topic.c_str());
}

// ── Smoke detection threshold ─────────────────────────────────────────────

void test_no_smoke_below_threshold() {
    Smoke smoke(SMOKE_PIN, 1, 2, "home/test");
    set_analog(SMOKE_PIN, 50);  // below default threshold of 70
    _mock_millis = 2001;
    smoke.loop(client);
    TEST_ASSERT_TRUE(client.wasPublished("home/test/smoke/notification", "NO SMOKE"));
}

void test_smoke_detected_above_threshold() {
    Smoke smoke(SMOKE_PIN, 1, 2, "home/test");
    set_analog(SMOKE_PIN, 80);  // above default threshold of 70
    _mock_millis = 2001;
    smoke.loop(client);
    TEST_ASSERT_TRUE(client.wasPublished("home/test/smoke/notification", "SMOKE DETECTED!!!"));
}

void test_custom_threshold_respected() {
    Smoke smoke(SMOKE_PIN, 1, 2, "home/test");
    smoke.setSmokeThreshold(90);
    set_analog(SMOKE_PIN, 80);  // below custom threshold of 90
    _mock_millis = 2001;
    smoke.loop(client);
    TEST_ASSERT_TRUE(client.wasPublished("home/test/smoke/notification", "NO SMOKE"));
}

// ── Check interval ────────────────────────────────────────────────────────

void test_no_publish_before_2s_interval() {
    Smoke smoke(SMOKE_PIN, 1, 2, "home/test");
    set_analog(SMOKE_PIN, 80);
    _mock_millis = 1000;  // only 1 s elapsed — check interval is 2 s
    smoke.loop(client);
    TEST_ASSERT_EQUAL(0, (int)client.published.size());
}

void test_publishes_after_2s_interval() {
    Smoke smoke(SMOKE_PIN, 1, 2, "home/test");
    set_analog(SMOKE_PIN, 80);
    _mock_millis = 2001;
    smoke.loop(client);
    TEST_ASSERT_GREATER_THAN(0, (int)client.published.size());
}

// ── processMessage ────────────────────────────────────────────────────────

void test_process_message_starts_test_mode() {
    Smoke smoke(SMOKE_PIN, 1, 2, "home/test");
    smoke.processMessage(client, "home/test/smoke", "Test/Reset");
    TEST_ASSERT_TRUE(client.wasPublished("home/test/smoke/notification", "IN TEST"));
}

void test_process_message_resets_when_triggered() {
    Smoke smoke(SMOKE_PIN, 1, 2, "home/test");
    // Trigger the alarm first
    set_analog(SMOKE_PIN, 80);
    _mock_millis = 2001;
    smoke.loop(client);
    client.reset();
    // Test/Reset while triggered → should reset, not enter test mode
    smoke.processMessage(client, "home/test/smoke", "Test/Reset");
    TEST_ASSERT_TRUE(client.wasPublished("home/test/smoke/notification", "NO SMOKE"));
}

void test_process_message_ignores_wrong_topic() {
    Smoke smoke(SMOKE_PIN, 1, 2, "home/test");
    smoke.processMessage(client, "home/other/topic", "Test/Reset");
    TEST_ASSERT_EQUAL(0, (int)client.published.size());
}

// ── Entry point ───────────────────────────────────────────────────────────

int main(int /*argc*/, char** /*argv*/) {
    UNITY_BEGIN();

    RUN_TEST(test_subscribe_uses_base_topic);
    RUN_TEST(test_report_status_uses_notification_topic);

    RUN_TEST(test_no_smoke_below_threshold);
    RUN_TEST(test_smoke_detected_above_threshold);
    RUN_TEST(test_custom_threshold_respected);

    RUN_TEST(test_no_publish_before_2s_interval);
    RUN_TEST(test_publishes_after_2s_interval);

    RUN_TEST(test_process_message_starts_test_mode);
    RUN_TEST(test_process_message_resets_when_triggered);
    RUN_TEST(test_process_message_ignores_wrong_topic);

    return UNITY_END();
}
