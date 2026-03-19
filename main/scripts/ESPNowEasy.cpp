#ifndef ESPNOWEASY_CPP
#define ESPNOWEASY_CPP

#include "ESPNowEasy.h"
#include <string.h>

static const char* TAG = "ESPNOW";

template<typename MessageType>
typename ESPNowEasy<MessageType>::ReceiverCallback ESPNowEasy<MessageType>::userCallback = nullptr;

template<typename MessageType>
int ESPNowEasy<MessageType>::lastRSSI = 0;

template<typename MessageType>
bool ESPNowEasy<MessageType>::begin(const uint8_t* peerAddr)
{
    // Initialize NVS (Required for WiFi)
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_start());

    if (esp_now_init() != ESP_OK) return false;

    // If we provided a MAC address, add it as a peer
    if (peerAddr != nullptr) {
        memcpy(targetAddress, peerAddr, 6);
        esp_now_peer_info_t peer = {};
        memcpy(peer.peer_addr, targetAddress, 6);
        peer.channel = 0;
        peer.encrypt = false;
        if (esp_now_add_peer(&peer) != ESP_OK) return false;
    }
    return true;
}

template<typename MessageType>
bool ESPNowEasy<MessageType>::send(MessageType& message) {
    return esp_now_send(targetAddress, (uint8_t*)&message, sizeof(MessageType)) == ESP_OK;
}

template<typename MessageType>
bool ESPNowEasy<MessageType>::addPeer(const uint8_t* peerAddr) {
    if (esp_now_is_peer_exist(peerAddr)) return true;
    esp_now_peer_info_t peer = {};
    memcpy(peer.peer_addr, peerAddr, 6);
    peer.channel = 0;
    peer.encrypt = false;
    return esp_now_add_peer(&peer) == ESP_OK;
}

template<typename MessageType>
template<typename T>
bool ESPNowEasy<MessageType>::sendAny(const uint8_t* addr, T& message) {
    return esp_now_send(addr, (uint8_t*)&message, sizeof(T)) == ESP_OK;
}

template<typename MessageType>
void ESPNowEasy<MessageType>::onReceive(ReceiverCallback callback) {
    userCallback = callback;
    esp_now_register_recv_cb(receiveCallback);
}

template<typename MessageType>
void ESPNowEasy<MessageType>::receiveCallback(const esp_now_recv_info_t *info, const uint8_t *data, int len) {
    if (len >= (int)sizeof(MessageType) && userCallback != nullptr) {
        lastRSSI = info->rx_ctrl->rssi;
        MessageType msg;
        memcpy(&msg, data, sizeof(MessageType));
        userCallback(msg, info->src_addr);
    }
}
#endif