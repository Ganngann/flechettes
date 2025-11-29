#include "MockArduino.h"

#ifndef ARDUINO

SerialMock Serial;
WiFiMock WiFi;

void pinMode(int pin, int mode) {}
void digitalWrite(int pin, int val) {}
int digitalRead(int pin) { return 0; }
void tone(int pin, unsigned int frequency, unsigned long duration) {}
void noTone(int pin) {}
unsigned long millis() { return 0; }
void delay(unsigned long ms) {}

void SerialMock::begin(unsigned long baud) {}
void SerialMock::println(const char* s) {}
void SerialMock::println(int n) {}
void SerialMock::print(const char* s) {}
void SerialMock::print(int n) {}

void WiFiMock::mode(int m) {}
void WiFiMock::disconnect() {}
const char* WiFiMock::macAddress() { return "00:00:00:00:00:00"; }

esp_err_t esp_now_init() { return ESP_OK; }
esp_err_t esp_now_add_peer(const esp_now_peer_info_t *peer) { return ESP_OK; }
esp_err_t esp_now_del_peer(const uint8_t *peer_addr) { return ESP_OK; }
bool esp_now_is_peer_exist(const uint8_t *peer_addr) { return false; }
esp_err_t esp_now_send(const uint8_t *peer_addr, const uint8_t *data, size_t len) { return ESP_OK; }
esp_err_t esp_now_register_recv_cb(esp_now_recv_cb_t cb) { return ESP_OK; }
esp_err_t esp_now_register_send_cb(esp_now_send_cb_t cb) { return ESP_OK; }

#endif
