// ESP-IDF bridge for ESP32-C3:
// Receives ESP-NOW packets and pretty-prints them to USB serial (UART0).

#include <stdio.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_wifi.h"
#include "esp_now.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"

#include "driver/uart.h"

static const char *TAG = "ESP_NOW_BRIDGE";

static void espnow_recv_cb(const esp_now_recv_info_t *recv_info,
    const uint8_t *data, int len)
{
char buf[300];

int copy_len = (len < (int)sizeof(buf) - 1) ? len : (int)sizeof(buf) - 1;
memcpy(buf, data, copy_len);
buf[copy_len] = '\0';

ESP_LOGI(TAG, "-----------------------------");

// Split into request line and telemetry line
char *req_line = buf;
char *telemetry_line = strchr(buf, '\n');
if (telemetry_line) {
*telemetry_line = '\0';
telemetry_line++;
}

if (!telemetry_line || *telemetry_line == '\0') {
telemetry_line = req_line;
req_line = NULL;
}

if (req_line) {
ESP_LOGI(TAG, "%s", req_line);
}

char *saveptr = NULL;
char *field = strtok_r(telemetry_line, ";", &saveptr);
while (field) {
while (*field == ' ') field++;
if (*field != '\0') {
ESP_LOGI(TAG, "%s", field);
}
field = strtok_r(NULL, ";", &saveptr);
}

ESP_LOGI(TAG, "");   // blank line after each packet
}

static void init_uart(void)
{
    const uart_config_t uart_config = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity    = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
    };
    ESP_ERROR_CHECK(uart_param_config(UART_NUM_0, &uart_config));
    // On ESP32-C3 devkits, UART0 is already mapped to USB serial, no pins to set
    ESP_ERROR_CHECK(uart_driver_install(UART_NUM_0, 1024, 0, 0, NULL, 0));
}

static void init_wifi_espnow(void)
{
    // Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ESP_ERROR_CHECK(nvs_flash_init());
    }

    // Initialize the underlying TCP/IP stack & event loop
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    // Wi-Fi init
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_start());

    // Print MAC address (STA interface)
    uint8_t mac[6];
    ESP_ERROR_CHECK(esp_wifi_get_mac(WIFI_IF_STA, mac));
    ESP_LOGI(TAG, "Bridge STA MAC: %02X:%02X:%02X:%02X:%02X:%02X",
             mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

    // Initialize ESP-NOW
    ESP_ERROR_CHECK(esp_now_init());
    ESP_ERROR_CHECK(esp_now_register_recv_cb(espnow_recv_cb));
    ESP_LOGI(TAG, "ESP-NOW initialized and receive callback registered");
}

void app_main(void)
{
    init_uart();
    init_wifi_espnow();

    ESP_LOGI(TAG, "ESP-NOW bridge running...");

    // Nothing else to do; ESP-NOW callbacks handle traffic
    while (true) {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
