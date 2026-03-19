#include "ESPNowEasy.h"
#include "esp_adc/adc_oneshot.h"
#include "hal/adc_types.h"
#include "esp_now.h"
#include <string.h>

struct Message {
    int pot_value;
};

struct SensorData {
    float temp_c;
    float humidity;
};

// Receive sensor telemetry sent back from the motor ESP
static void on_sensor_data(const esp_now_recv_info_t *info, const uint8_t *data, int len) {
    if (len == sizeof(SensorData)) {
        SensorData s;
        memcpy(&s, data, sizeof(s));
        printf("Sensor <- %.2f C  %.2f%%RH | RSSI: %d dBm\n", s.temp_c, s.humidity, info->rx_ctrl->rssi);
    }
}

// Target MAC (The Drone's MAC)
uint8_t drone_mac[] = {0x64, 0xE8, 0x33, 0xDC, 0xA2, 0xDC}; 

ESPNowEasy<Message> espNow;

extern "C" void app_main(void) {
    // 1. Start ESP-NOW
    espNow.begin(drone_mac);
    esp_now_register_recv_cb(on_sensor_data);
    
    // 2. Setup ADC (v5 style) - Fixed order and initializers
    adc_oneshot_unit_handle_t adc1_handle;
    adc_oneshot_unit_init_cfg_t init_config1 = {}; 
    init_config1.unit_id = ADC_UNIT_1;
    init_config1.ulp_mode = ADC_ULP_MODE_DISABLE;
    adc_oneshot_new_unit(&init_config1, &adc1_handle);

    adc_oneshot_chan_cfg_t config = {};
    config.atten = ADC_ATTEN_DB_12;
    config.bitwidth = ADC_BITWIDTH_12;
    adc_oneshot_config_channel(adc1_handle, ADC_CHANNEL_2, &config);

    Message data;
    while (1) {
        // Read Potentiometer on GPIO 2 (ADC1 Channel 2)
        adc_oneshot_read(adc1_handle, ADC_CHANNEL_2, &data.pot_value);
        
        // Send to Drone
        espNow.send(data);
        //printf("TX -> Sending Pot: %d\n", data.pot_value);
        
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}
