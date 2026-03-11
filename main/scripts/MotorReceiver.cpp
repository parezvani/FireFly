#include "ESPNowEasy.h"
#include "driver/gpio.h"
#include "driver/ledc.h"

// Use typed gpio_num_t constants instead of plain int macros to match ESP-IDF API
static const gpio_num_t ENA_PWM_PIN = GPIO_NUM_3;
static const gpio_num_t IN1_PIN = GPIO_NUM_4;
static const gpio_num_t IN2_PIN = GPIO_NUM_5;

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
    gpio_set_direction(IN1_PIN, GPIO_MODE_OUTPUT);
    gpio_set_direction(IN2_PIN, GPIO_MODE_OUTPUT);

    // 2. Setup PWM (LEDC)
    ledc_timer_config_t timer = {}; // zero-initialize to avoid missing-field warnings
    timer.speed_mode = LEDC_LOW_SPEED_MODE;
    timer.duty_resolution = LEDC_TIMER_12_BIT;
    timer.timer_num = LEDC_TIMER_0;
    timer.freq_hz = 1000;
    timer.clk_cfg = LEDC_AUTO_CLK;
    ledc_timer_config(&timer);

    ledc_channel_config_t chan = {}; // zero-initialize to avoid missing-field warnings
    chan.gpio_num = ENA_PWM_PIN;
    chan.speed_mode = LEDC_LOW_SPEED_MODE;
    chan.channel = LEDC_CHANNEL_0;
    chan.timer_sel = LEDC_TIMER_0;
    chan.duty = 0;
    ledc_channel_config(&chan);

    // 3. Start ESP-NOW
    espNow.begin();
    espNow.onReceive(handleIncoming);
}