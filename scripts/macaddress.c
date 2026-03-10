#include "esp_mac.h"
#include "esp_log.h"

void app_main(void)
{
    uint8_t mac[6];

    esp_read_mac(mac, ESP_MAC_WIFI_STA);

    ESP_LOGI("MAC",
        "ESP32 MAC: %02X:%02X:%02X:%02X:%02X:%02X",
        mac[0], mac[1], mac[2],
        mac[3], mac[4], mac[5]);
}