#include <stdio.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "driver/gpio.h"
#include "esp_log.h"

#include "nimble/nimble_port.h"
#include "nimble/nimble_port_freertos.h"
#include "host/ble_hs.h"
#include "esp_nimble_hci.h"
#include "services/gap/ble_svc_gap.h"
#include "services/gatt/ble_svc_gatt.h"

#define LED_PIN 7
#define TAG "BLE_BLINK"

// Blink control variable
volatile bool blink_enabled = false;

// BLE characteristic handle
static uint16_t rx_handle; // GATT handle, nimble fills in. automatically

// When phone sends data
static int ble_rx_cb(uint16_t conn_handle, uint16_t attr_handle,
                     struct ble_gatt_access_ctxt *ctxt, void *arg)
{
    char incoming[32] = {0};
    memcpy(incoming, ctxt->om->om_data, ctxt->om->om_len);

    ESP_LOGI(TAG, "Received: %s", incoming);

    if (strcmp(incoming, "BLINK") == 0) {
        blink_enabled = true;
        ESP_LOGI(TAG, "Blink enabled");
    } else if (strcmp(incoming, "STOP") == 0) {
        blink_enabled = false;
        ESP_LOGI(TAG, "Blink disabled");
        gpio_set_level(LED_PIN, 0);
    }

    return 0;
}

// BLE GATT Service (generic attribute profile for services & characteristics)
static const struct ble_gatt_svc_def gatt_svr_svcs[] = {
    {
        .type = BLE_GATT_SVC_TYPE_PRIMARY,
        .uuid = BLE_UUID16_DECLARE(0xAB00), // custom service
        .characteristics = (struct ble_gatt_chr_def[]){
            {
                .uuid = BLE_UUID16_DECLARE(0xAB01), // custom writeable characteristic
                .flags = BLE_GATT_CHR_F_WRITE, // allows phone to write
                .access_cb = ble_rx_cb,
                .val_handle = &rx_handle,
            },
            {0},
        },
    },
    {0},
};

// GAP Event Handler (general access profile for advertising, connecting)
static int gap_event(struct ble_gap_event *event, void *arg)
{
    return 0;
}

// BLE initialization
void ble_app_on_sync(void)
{
    ble_hs_id_infer_auto(0, NULL); // choose best BLE address

    struct ble_gap_adv_params adv_params = {0};
    adv_params.conn_mode = BLE_GAP_CONN_MODE_UND;
    adv_params.disc_mode = BLE_GAP_DISC_MODE_GEN;

    ble_gap_adv_start(0, NULL, BLE_HS_FOREVER, &adv_params, gap_event, NULL); // start advertising
}

// nimble host task (boilerplate, runs bluetooth stack in RreeRTOS task)
void ble_host_task(void *param)
{
    nimble_port_run();
    nimble_port_freertos_deinit();
}

void app_main(void)
{
    // Configure LED
    gpio_reset_pin(LED_PIN);
    gpio_set_direction(LED_PIN, GPIO_MODE_OUTPUT);

    // Start NimBLE
    esp_nimble_hci_and_controller_init();
    nimble_port_init();

    ble_svc_gap_init();
    ble_svc_gatt_init();
    ble_gatts_count_cfg(gatt_svr_svcs);
    ble_gatts_add_svcs(gatt_svr_svcs);
    ble_svc_gap_device_name_set("ESP32C3-BLINK");

    nimble_port_freertos_init(ble_host_task);

    ESP_LOGI(TAG, "Ready. Use nRF Connect to write BLINK or STOP.");

    // Blink loop
    bool led_state = 0;

    while (1) {
        if (blink_enabled) {
            led_state = !led_state;
            gpio_set_level(LED_PIN, led_state);
            vTaskDelay(500 / portTICK_PERIOD_MS);
        } else {
            vTaskDelay(100 / portTICK_PERIOD_MS);
        }
    }
}