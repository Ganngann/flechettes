#ifndef NETWORK_MANAGER_H
#define NETWORK_MANAGER_H

#include "utils.h"
#ifdef ARDUINO
#include <esp_now.h>
#include <WiFi.h>
#include <esp_idf_version.h>
#endif
#include "SharedData.h"
#include <functional>

class NetworkManager {
public:
    static NetworkManager& getInstance();

    void init(const uint8_t* peerAddress, uint8_t channel = 0);
    bool sendData(const struct_message& data);

    // Callbacks
    using OnReceiveCallback = std::function<void(const struct_message&, const uint8_t* mac)>;
    using OnSendCallback = std::function<void(bool success)>;

    void setOnReceiveCallback(OnReceiveCallback cb);
    void setOnSendCallback(OnSendCallback cb);

private:
    NetworkManager();

    // Static ESP-NOW callbacks
#if ESP_IDF_VERSION_MAJOR >= 5
    static void onDataSentStatic(const wifi_tx_info_t *tx_info, esp_now_send_status_t status);
#else
    static void onDataSentStatic(const uint8_t *mac_addr, esp_now_send_status_t status);
#endif
    static void onDataRecvStatic(const uint8_t *mac, const uint8_t *incomingData, int len);

    OnReceiveCallback _onReceive;
    OnSendCallback _onSend;
    uint8_t _peerAddress[6];
    bool _initialized;
};

#endif
