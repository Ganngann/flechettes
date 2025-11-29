#ifndef MOCK_ARDUINO_H
#define MOCK_ARDUINO_H

#ifndef ARDUINO
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <string>

#define OUTPUT 1
#define INPUT 0
#define LOW 0
#define HIGH 1
#define INPUT_PULLUP 2

void pinMode(int pin, int mode);
void digitalWrite(int pin, int val);
int digitalRead(int pin);
void tone(int pin, unsigned int frequency, unsigned long duration = 0);
void noTone(int pin);
unsigned long millis();
void delay(unsigned long ms);

class SerialMock {
public:
    void begin(unsigned long baud);
    void println(const char* s);
    void println(int n);
    void print(const char* s);
    void print(int n);
};
extern SerialMock Serial;

#define WIFI_STA 1
class WiFiMock {
public:
    void mode(int m);
    void disconnect();
    const char* macAddress();
};
extern WiFiMock WiFi;

typedef int esp_err_t;
#define ESP_OK 0
#define ESP_FAIL -1

typedef struct esp_now_peer_info {
    uint8_t peer_addr[6];
    uint8_t channel;
    uint8_t id;
    bool encrypt;
    void* lmk;
} esp_now_peer_info_t;

typedef int esp_now_send_status_t;
#define ESP_NOW_SEND_SUCCESS 0

typedef void (*esp_now_recv_cb_t)(const uint8_t *mac_addr, const uint8_t *data, int len);
typedef void (*esp_now_send_cb_t)(const uint8_t *mac_addr, esp_now_send_status_t status);

// For ESP_IDF_VERSION macros
#define ESP_IDF_VERSION_MAJOR 4

// Fake TX info for version 5 logic (if needed)
typedef struct wifi_tx_info {
} wifi_tx_info_t;

esp_err_t esp_now_init();
esp_err_t esp_now_add_peer(const esp_now_peer_info_t *peer);
esp_err_t esp_now_del_peer(const uint8_t *peer_addr);
bool esp_now_is_peer_exist(const uint8_t *peer_addr);
esp_err_t esp_now_send(const uint8_t *peer_addr, const uint8_t *data, size_t len);
esp_err_t esp_now_register_recv_cb(esp_now_recv_cb_t cb);
esp_err_t esp_now_register_send_cb(esp_now_send_cb_t cb);

#endif // !ARDUINO
#endif // MOCK_ARDUINO_H
