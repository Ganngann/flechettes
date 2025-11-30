#include <unity.h>
#include "utils.h"
#include "GameLogic.h"

// === UTILS TESTS ===
void test_format_mac_address(void) {
    uint8_t mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};
    String result = formatMacAddress(mac);
    TEST_ASSERT_EQUAL_STRING("DE:AD:BE:EF:FE:ED", result.c_str());
}

// === GAME LOGIC TESTS ===
GameLogic::Config testConfig = {
    100, // relayPulseDuration
    1000 // delayBetweenPulses
};

void test_receive_credit(void) {
    GameLogic logic(testConfig);
    GameLogic::Input in = {0};
    in.currentMillis = 10; // Not enough time to trigger first pulse (requires 1000ms)
    in.dataReceived = true;
    in.receivedData.cp1 = 2;

    GameLogic::Output out = logic.update(in);

    TEST_ASSERT_EQUAL(2, logic.getPendingCredits());
    TEST_ASSERT_TRUE(out.updateScreen);
    TEST_ASSERT_FALSE(out.relayActive);
}

void test_process_credit(void) {
    GameLogic logic(testConfig);
    logic.setStats(1, 0, 0, 0); // 1 pending credit

    GameLogic::Input in = {0};
    in.currentMillis = 2000; // ample time

    GameLogic::Output out = logic.update(in);

    // Should trigger relay
    TEST_ASSERT_TRUE(out.relayActive);
    // 1 pending -> 0 pending. Last one triggers Sound 5.
    TEST_ASSERT_EQUAL(5, out.soundToPlay);
    TEST_ASSERT_EQUAL(1, logic.getTotalCredits());
    TEST_ASSERT_EQUAL(0, logic.getPendingCredits()); // Decremented
}

void test_process_multiple_credits(void) {
    GameLogic logic(testConfig);
    logic.setStats(2, 0, 0, 0); // 2 pending

    GameLogic::Input in = {0};
    in.currentMillis = 2000;

    // 1st Credit
    GameLogic::Output out = logic.update(in);
    TEST_ASSERT_TRUE(out.relayActive);
    TEST_ASSERT_EQUAL(0, out.soundToPlay); // Not the last one
    TEST_ASSERT_EQUAL(1, logic.getPendingCredits());

    // Wait for delay
    in.currentMillis += 1000;
    // We also need relay to be off, handled by separate update usually, but update() checks logic
    // Logic says: if (!relayActive ...)
    // So we must simulate relay turning off first
    in.currentMillis += 10; // 2010. Relay on for 100ms.
    // Wait... we need to turn it off first.

    // Fast forward to relay off time
    in.currentMillis = 2000 + 100;
    logic.update(in); // Relay turns off

    // Now fast forward to next pulse time (2000 + 1000)
    in.currentMillis = 3000;
    out = logic.update(in);

    TEST_ASSERT_TRUE(out.relayActive);
    TEST_ASSERT_EQUAL(5, out.soundToPlay); // Last one
    TEST_ASSERT_EQUAL(0, logic.getPendingCredits());
}

void test_relay_timing(void) {
    GameLogic logic(testConfig);
    logic.setStats(1, 0, 0, 0);

    GameLogic::Input in = {0};
    in.currentMillis = 2000;

    // Trigger
    GameLogic::Output out = logic.update(in);
    TEST_ASSERT_TRUE(out.relayActive);

    // Advance time < duration
    in.currentMillis += 50;
    out = logic.update(in);
    TEST_ASSERT_TRUE(out.relayActive);

    // Advance time > duration
    in.currentMillis += 60; // Total 110 > 100
    out = logic.update(in);
    TEST_ASSERT_FALSE(out.relayActive);
}

void setUp(void) {}
void tearDown(void) {}

int main(int argc, char **argv) {
    UNITY_BEGIN();
    RUN_TEST(test_format_mac_address);
    RUN_TEST(test_receive_credit);
    RUN_TEST(test_process_credit);
    RUN_TEST(test_process_multiple_credits);
    RUN_TEST(test_relay_timing);
    UNITY_END();
    return 0;
}
