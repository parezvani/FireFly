#ifndef ESPNOWEASY_CPP
#define ESPNOWEASY_CPP

#include "ESPNowEasy.h"
#include <string.h>

static const char* TAG = "ESPNOW";

template<typename MessageType>
typename ESPNowEasy<MessageType>::ReceiverCallback
ESPNowEasy<MessageType>::userCallback = nullptr;


template<typename MessageType>
bool ESPNowEasy<MessageType>::begin()
{
    ESP_LOGI(TAG, "Initializing WiFi + ESP-NOW");

    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_start());

    esp_wifi_set_protocol(WIFI_IF_STA, WIFI_PROTOCOL_LR);

    if (esp_now_init() != ESP_OK)
    {
        ESP_LOGE(TAG, "ESP-NOW init failed");
        return false;
    }

    esp_now_peer_info_t peer = {};
    memcpy(peer.peer_addr, broadcastAddress, 6);
    peer.channel = 0;
    peer.encrypt = false;

    if (esp_now_add_peer(&peer) != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to add broadcast peer");
        return false;
    }

    ESP_LOGI(TAG, "ESP-NOW initialized");

    return true;
}


template<typename MessageType>
bool ESPNowEasy<MessageType>::send(MessageType& message)
{
    esp_err_t result =
        esp_now_send(broadcastAddress,
        (uint8_t*)&message,
        sizeof(MessageType));

    if (result == ESP_OK)
    {
        ESP_LOGI(TAG, "Message sent");
        return true;
    }

    ESP_LOGE(TAG, "Send failed");
    return false;
}


template<typename MessageType>
void ESPNowEasy<MessageType>::onReceive(ReceiverCallback callback)
{
    userCallback = callback;
    esp_now_register_recv_cb(receiveCallback);
}


template<typename MessageType>
void ESPNowEasy<MessageType>::receiveCallback(
        const esp_now_recv_info_t *info,
        const uint8_t *data,
        int len)
{
    if (len < sizeof(MessageType))
        return;

    if (userCallback == nullptr)
        return;

    MessageType message;
    memcpy(&message, data, sizeof(MessageType));

    userCallback(message);
}

#endif