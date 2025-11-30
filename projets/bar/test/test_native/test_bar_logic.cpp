#include <unity.h>
#include "utils.h"
#include "BarLogic.h"

// === UTILS TESTS ===
void test_format_mac_address(void) {
    uint8_t mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};
    String result = formatMacAddress(mac);
    TEST_ASSERT_EQUAL_STRING("DE:AD:BE:EF:FE:ED", result.c_str());
}

// === BAR LOGIC TESTS ===
BarLogic::Config testConfig = {
    1, // valTotCn
    10, // maxCred
    500 // sendTimeout
};

void test_initial_state(void) {
    BarLogic logic(testConfig);
    TEST_ASSERT_EQUAL(BarLogic::ETAT_INIT, logic.getState());

    BarLogic::Input in = {0};
    in.currentMillis = 100;

    BarLogic::Output out = logic.update(in);

    // Should transition to ATTENTE immediately
    TEST_ASSERT_EQUAL(BarLogic::ETAT_ATTENTE, logic.getState());
    TEST_ASSERT_EQUAL(1, out.screenId);
}

void test_add_credit(void) {
    BarLogic logic(testConfig);
    // Move to ATTENTE
    BarLogic::Input in = {0};
    in.currentMillis = 100;
    logic.update(in);

    in.currentMillis += 100;
    in.isConnected = true;
    in.btn2Down = true; // Press

    BarLogic::Output out = logic.update(in);

    // Debounce wait... (need > 50ms)
    in.currentMillis += 60;

    // Release
    in.btn2Down = false;
    out = logic.update(in); // Logic triggers on Release

    TEST_ASSERT_EQUAL(1, logic.getTotalCredits());
    TEST_ASSERT_EQUAL(1, out.soundToPlay);
    TEST_ASSERT_EQUAL(3, out.screenId);
}

void test_send_credit(void) {
    BarLogic logic(testConfig);
    logic.setCredits(5);
    BarLogic::Input in = {0};
    in.currentMillis = 1000;
    // Move to ATTENTE (from init)
    logic.update(in); // Init -> Attente

    // Press B3
    in.currentMillis += 100;
    in.btn3Down = true;

    BarLogic::Output out = logic.update(in);
    // Should detect press immediately because time > debounce (0)

    TEST_ASSERT_EQUAL(BarLogic::ETAT_ENVOI_CREDIT, logic.getState());

    // Next cycle triggers send
    in.currentMillis += 10;
    out = logic.update(in);

    TEST_ASSERT_TRUE(out.sendMessage);
    TEST_ASSERT_EQUAL(5, out.messageData.cp1);
    TEST_ASSERT_EQUAL(BarLogic::ETAT_RESULTAT_ENVOI, logic.getState());

    // Wait for timeout (500ms)
    in.currentMillis += 550;
    in.sendSuccess = true;
    out = logic.update(in);

    TEST_ASSERT_EQUAL(0, logic.getTotalCredits());
    TEST_ASSERT_EQUAL(BarLogic::ETAT_TIMEOUT_ECRAN, logic.getState());
    TEST_ASSERT_EQUAL(20, out.screenId); // Success ID
}

void setUp(void) {}
void tearDown(void) {}

int main(int argc, char **argv) {
    UNITY_BEGIN();
    RUN_TEST(test_format_mac_address);
    RUN_TEST(test_initial_state);
    RUN_TEST(test_add_credit);
    RUN_TEST(test_send_credit);
    UNITY_END();
    return 0;
}
