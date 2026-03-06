#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/ledc.h"
#include "esp_adc/adc_oneshot.h" // The new v5+ ADC header

// Define your pins
#define POTENTIOMETER_CHANNEL ADC_CHANNEL_2 // GPIO2 is ADC1 Channel 2
#define ENA_PWM_PIN           3
#define IN1_PIN               4
#define IN2_PIN               5

void app_main(void)
{
    // 1. Configure Direction Pins (IN1 & IN2)
    gpio_reset_pin(IN1_PIN);
    gpio_reset_pin(IN2_PIN);
    gpio_set_direction(IN1_PIN, GPIO_MODE_OUTPUT);
    gpio_set_direction(IN2_PIN, GPIO_MODE_OUTPUT);

    // Set motor to spin "Forward"
    gpio_set_level(IN1_PIN, 1);
    gpio_set_level(IN2_PIN, 0);

    // 2. Configure PWM for ENA Pin using LEDC
    ledc_timer_config_t ledc_timer = {
        .speed_mode       = LEDC_LOW_SPEED_MODE,
        .timer_num        = LEDC_TIMER_0,
        .duty_resolution  = LEDC_TIMER_12_BIT, // 0-4095
        .freq_hz          = 1000,
        .clk_cfg          = LEDC_AUTO_CLK
    };
    ledc_timer_config(&ledc_timer);

    ledc_channel_config_t ledc_channel = {
        .speed_mode     = LEDC_LOW_SPEED_MODE,
        .channel        = LEDC_CHANNEL_0,
        .timer_sel      = LEDC_TIMER_0,
        .intr_type      = LEDC_INTR_DISABLE,
        .gpio_num       = ENA_PWM_PIN,
        .duty           = 0,
        .hpoint         = 0
    };
    ledc_channel_config(&ledc_channel);

    // 3. Configure ADC (The New v5+ One-Shot API)
    adc_oneshot_unit_handle_t adc1_handle;
    adc_oneshot_unit_init_cfg_t init_config1 = {
        .unit_id = ADC_UNIT_1,
    };
    adc_oneshot_new_unit(&init_config1, &adc1_handle);

    adc_oneshot_chan_cfg_t config = {
        .bitwidth = ADC_BITWIDTH_12,
        .atten = ADC_ATTEN_DB_12, // DB_12 is standard in v5 to read full 0-3.3V range
    };
    adc_oneshot_config_channel(adc1_handle, POTENTIOMETER_CHANNEL, &config);

    // 4. The Main Control Loop
    while (1) {
        int pot_value = 0;
        adc_oneshot_read(adc1_handle, POTENTIOMETER_CHANNEL, &pot_value);

        int duty = 0;
        int center_point = 2047; 
        int deadband = 150;      
        
        // Simplified state tracker
        const char* direction_str = "Stopped";

        // -- ZONE 1: FORWARD --
        if (pot_value > (center_point + deadband)) {
            gpio_set_level(IN1_PIN, 1);
            gpio_set_level(IN2_PIN, 0);
            
            int active_range = 4095 - (center_point + deadband);
            int current_position = pot_value - (center_point + deadband);
            duty = (current_position * 4095) / active_range;
            
            direction_str = "Forward";
        } 
        
        // -- ZONE 2: BACKWARDS --
        else if (pot_value < (center_point - deadband)) {
            gpio_set_level(IN1_PIN, 0);
            gpio_set_level(IN2_PIN, 1);
            
            int active_range = center_point - deadband;
            int current_position = active_range - pot_value; 
            duty = (current_position * 4095) / active_range;
            
            direction_str = "Backwards";
        } 
        
        // -- ZONE 3: STOP (The Deadband) --
        else {
            gpio_set_level(IN1_PIN, 0);
            gpio_set_level(IN2_PIN, 0);
            duty = 0;
            
            direction_str = "Stopped";
        }

        // Safety cap
        if (duty > 4095) duty = 4095;
        if (duty < 0) duty = 0;

        // Apply the speed
        ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, duty);
        ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);

        // The ultra-clean serial monitor output
        printf("Direction: %s\n", direction_str);

        // Yield to FreeRTOS
        vTaskDelay(pdMS_TO_TICKS(50)); 
    }
}