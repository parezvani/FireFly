#include "ESPNowEasy.h"
#include "driver/gpio.h"
#include "driver/ledc.h"

#define ENA_PWM_PIN 3
#define IN1_PIN     4
#define IN2_PIN     5

struct Message {
    int pot_value;
};

ESPNowEasy<Message> espNow;

void handleIncoming(Message& msg) {
    int duty = 0;
    int center_point = 2047; 
    int deadband = 150;

    if (msg.pot_value > (center_point + deadband)) {
        gpio_set_level(IN1_PIN, 1);
        gpio_set_level(IN2_PIN, 0);
        duty = ((msg.pot_value - (center_point + deadband)) * 4095) / (4095 - (center_point + deadband));
    } else if (msg.pot_value < (center_point - deadband)) {
        gpio_set_level(IN1_PIN, 0);
        gpio_set_level(IN2_PIN, 1);
        duty = ((center_point - deadband - msg.pot_value) * 4095) / (center_point - deadband);
    } else {
        gpio_set_level(IN1_PIN, 0);
        gpio_set_level(IN2_PIN, 0);
        duty = 0;
    }

    ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, duty);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);
    printf("Received Pot: %d | Duty: %d\n", msg.pot_value, duty);
}

extern "C" void app_main(void) {
    // 1. Setup Motor Pins (GPIO 4, 5)
    gpio_set_direction((gpio_num_t)IN1_PIN, GPIO_MODE_OUTPUT);
    gpio_set_direction((gpio_num_t)IN2_PIN, GPIO_MODE_OUTPUT);

    // 2. Setup PWM (LEDC)
    ledc_timer_config_t timer = { .speed_mode = LEDC_LOW_SPEED_MODE, .duty_resolution = LEDC_TIMER_12_BIT, .timer_num = LEDC_TIMER_0, .freq_hz = 1000, .clk_cfg = LEDC_AUTO_CLK };
    ledc_timer_config(&timer);
    ledc_channel_config_t chan = { .gpio_num = ENA_PWM_PIN, .speed_mode = LEDC_LOW_SPEED_MODE, .channel = LEDC_CHANNEL_0, .timer_sel = LEDC_TIMER_0, .duty = 0 };
    ledc_channel_config(&chan);

    // 3. Start ESP-NOW
    espNow.begin();
    espNow.onReceive(handleIncoming);
}