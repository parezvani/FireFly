#include "ESPNowEasy.h"

struct Message {
    int pot_value; // Sending the raw ADC reading
};

// Use the MAC address of your Motor ESP here
uint8_t motor_esp_mac[] = {0x64, 0xE8, 0x33, 0xDC, 0xA2, 0xDC}; 

ESPNowEasy<Message> espNow;

extern "C" void app_main(void) {
    espNow.begin(motor_esp_mac);
    
    // Setup ADC (Code from your motor.c)
    adc_oneshot_unit_handle_t adc1_handle;
    adc_oneshot_unit_init_cfg_t init_config1 = { .unit_id = ADC_UNIT_1 };
    adc_oneshot_new_unit(&init_config1, &adc1_handle);
    adc_oneshot_chan_cfg_t config = { .bitwidth = ADC_BITWIDTH_12, .atten = ADC_ATTEN_DB_12 };
    adc_oneshot_config_channel(adc1_handle, ADC_CHANNEL_2, &config);

    Message data;
    while (1) {
        adc_oneshot_read(adc1_handle, ADC_CHANNEL_2, &data.pot_value);
        espNow.send(data);
        printf("Sending Pot Value: %d\n", data.pot_value);
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}