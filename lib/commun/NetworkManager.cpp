#include "NetworkManager.h"

NetworkManager& NetworkManager::getInstance() {
    static NetworkManager instance;
    return instance;
}

NetworkManager::NetworkManager() : _initialized(false) {}

void NetworkManager::init(const uint8_t* peerAddress, uint8_t channel) {
    if (_initialized) return;

    WiFi.mode(WIFI_STA);
    WiFi.disconnect(); // Bonnes pratiques pour ESP-NOW

    if (esp_now_init() != ESP_OK) {
        Serial.println("Error initializing ESP-NOW");
        return;
    }

    esp_now_register_send_cb(onDataSentStatic);
    esp_now_register_recv_cb(onDataRecvStatic);

    memcpy(_peerAddress, peerAddress, 6);

    esp_now_peer_info_t peerInfo;
    memset(&peerInfo, 0, sizeof(peerInfo));
    memcpy(peerInfo.peer_addr, _peerAddress, 6);
    peerInfo.channel = channel;
    peerInfo.encrypt = false;

    if (esp_now_is_peer_exist(_peerAddress)) {
        // Optionnel : supprimer si on veut être sûr de la config, ou laisser tel quel
        // esp_now_del_peer(_peerAddress);
    }

    if (!esp_now_is_peer_exist(_peerAddress)) {
        if (esp_now_add_peer(&peerInfo) != ESP_OK) {
            Serial.println("Failed to add peer");
            return;
        }
    }

    _initialized = true;
}

bool NetworkManager::sendData(const struct_message& data) {
    if (!_initialized) return false;
    esp_err_t result = esp_now_send(_peerAddress, (uint8_t *)&data, sizeof(data));
    return result == ESP_OK;
}

void NetworkManager::setOnReceiveCallback(OnReceiveCallback cb) {
    _onReceive = cb;
}

void NetworkManager::setOnSendCallback(OnSendCallback cb) {
    _onSend = cb;
}

#if ESP_IDF_VERSION_MAJOR >= 5
void NetworkManager::onDataSentStatic(const wifi_tx_info_t *tx_info, esp_now_send_status_t status) {
    bool success = (status == ESP_NOW_SEND_SUCCESS);
    if (getInstance()._onSend) {
        getInstance()._onSend(success);
    }
}
#else
void NetworkManager::onDataSentStatic(const uint8_t *mac_addr, esp_now_send_status_t status) {
    bool success = (status == ESP_NOW_SEND_SUCCESS);
    if (getInstance()._onSend) {
        getInstance()._onSend(success);
    }
}
#endif

void NetworkManager::onDataRecvStatic(const uint8_t *mac, const uint8_t *incomingData, int len) {
    if (len != sizeof(struct_message)) return;

    struct_message data;
    memcpy(&data, incomingData, sizeof(data));

    if (getInstance()._onReceive) {
        getInstance()._onReceive(data, mac);
    }
}
