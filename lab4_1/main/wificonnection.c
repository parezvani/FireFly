#include <string.h>
#include <sys/param.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"

#include "driver/gpio.h"
#include "lwip/sockets.h"
#include "lwip/netdb.h"

#define WIFI_SSID      "ESP-DRONE_DEMO"
#define WIFI_PASS      "12345678"

#define UDP_PORT       11111
#define LED_GPIO       7

static const char *TAG = "esp_drone_demo";

static void wifi_init_softap(void) {
    ESP_LOGI(TAG, "Starting Wi-Fi Access Point");

    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    esp_netif_create_default_wifi_ap();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    wifi_config_t wifi_config = {
        .ap = {
            .ssid = WIFI_SSID,
            .ssid_len = strlen(WIFI_SSID),
            .password = WIFI_PASS,
            .max_connection = 4,
            .authmode = WIFI_AUTH_WPA_WPA2_PSK,
        },
    };

    if (strlen(WIFI_PASS) == 0) {
        wifi_config.ap.authmode = WIFI_AUTH_OPEN;
    }

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "Wi-Fi AP started. SSID=%s", WIFI_SSID);
}

static void udp_server_task(void *pvParameters) {
    char rx_buffer[128];
    struct sockaddr_in server_addr;
    struct sockaddr_in source_addr;

    ESP_LOGI(TAG, "UDP task started");

    int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
    if (sock < 0) {
        ESP_LOGE(TAG, "Unable to create socket");
        vTaskDelete(NULL);
        return;
    }

    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(UDP_PORT);

    if (bind(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        ESP_LOGE(TAG, "Socket bind failed");
        close(sock);
        vTaskDelete(NULL);
        return;
    }

    ESP_LOGI(TAG, "UDP server listening on port %d", UDP_PORT);

    while (1) {
        socklen_t socklen = sizeof(source_addr); // reset for each recvfrom
        int len = recvfrom(sock, rx_buffer, sizeof(rx_buffer) - 1, 0,
                           (struct sockaddr *)&source_addr, &socklen);

        if (len < 0) {
            ESP_LOGW(TAG, "recvfrom failed, errno=%d", errno);
            vTaskDelay(pdMS_TO_TICKS(100)); // avoid busy loop
            continue;
        } else if (len == 0) {
            ESP_LOGI(TAG, "recvfrom returned 0 bytes");
            continue;
        }

        rx_buffer[len] = 0; // null-terminate
        ESP_LOGI(TAG, "Received UDP packet (%d bytes) from %s:%d: %s",
                 len,
                 inet_ntoa(source_addr.sin_addr),
                 ntohs(source_addr.sin_port),
                 rx_buffer);

        // Toggle LED on packet receive
        int current = gpio_get_level(LED_GPIO);
        gpio_set_level(LED_GPIO, !current);
    }

    // never reached
    close(sock);
    vTaskDelete(NULL);
}

void app_main(void) {
    ESP_ERROR_CHECK(nvs_flash_init());

    // LED setup
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << LED_GPIO),
        .mode = GPIO_MODE_OUTPUT,
    };
    gpio_config(&io_conf);
    gpio_set_level(LED_GPIO, 0);

    wifi_init_softap();

    xTaskCreate(udp_server_task, "udp_server", 4096, NULL, 5, NULL);
}