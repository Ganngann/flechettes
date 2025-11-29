#include <unity.h>
#include "utils.h"

void setUp(void) {
}

void tearDown(void) {
}

void test_format_mac_address(void) {
    uint8_t mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};
    String result = formatMacAddress(mac);
    TEST_ASSERT_EQUAL_STRING("DE:AD:BE:EF:FE:ED", result.c_str());
}

int main(int argc, char **argv) {
    UNITY_BEGIN();
    RUN_TEST(test_format_mac_address);
    UNITY_END();

    return 0;
}
