#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"

#define MOTOR_PIN 7

void app_main(void)
{
    // 1. Reset the pin to a known state and configure it as an Output
    gpio_reset_pin(MOTOR_PIN);
    gpio_set_direction(MOTOR_PIN, GPIO_MODE_OUTPUT);

    // Make sure the Motor is off
    gpio_set_level(MOTOR_PIN, 0);


    while (1) {
        printf("Turning Motor ON for 5 seconds...\n");
        
        // Turn the Motor ON 
        gpio_set_level(MOTOR_PIN, 1);
        
        // Wait for 5 seconds 
        vTaskDelay(5000 / portTICK_PERIOD_MS);
        
        // Turn the Motor OFF 
        gpio_set_level(MOTOR_PIN, 0);
        
        printf("Motor turned off. Waiting 2 seconds...\n");

        // Wait for 2 seconds while the Motor is off before looping back
        vTaskDelay(2000 / portTICK_PERIOD_MS);
    }
}