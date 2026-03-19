#include "ESPNowEasy.h"
#include "driver/gpio.h"
#include "driver/ledc.h"
#include "driver/i2c.h"
#include <string.h>

// ---------- Motor pins ----------
static const gpio_num_t ENA_PWM_PIN = GPIO_NUM_3;
static const gpio_num_t IN1_PIN     = GPIO_NUM_4;
static const gpio_num_t IN2_PIN     = GPIO_NUM_5;

// ---------- Message structs ----------
struct Message {
    int pot_value;
};

struct SensorData {
    float temp_c;
    float humidity;
};

// ---------- Remote sender MAC ----------
static uint8_t remote_mac[] = {0xA0, 0x85, 0xE3, 0x07, 0x0A, 0x78};

ESPNowEasy<Message> espNow;

// ---------- SHTC3 config ----------
#define I2C_PORT            I2C_NUM_0
#define I2C_SDA_IO          GPIO_NUM_10
#define I2C_SCL_IO          GPIO_NUM_8
#define I2C_FREQ_HZ         100000
#define SHTC3_ADDR          0x70
#define SHTC3_CMD_WAKEUP    0x3517
#define SHTC3_CMD_SLEEP     0xB098
#define SHTC3_CMD_MEAS_T    0x7CA2   // temperature first
#define SHTC3_CMD_MEAS_RH   0x5C24   // humidity first

// ---------- SHTC3 helpers ----------
static uint8_t shtc3_crc(uint8_t *data, size_t len) {
    uint8_t crc = 0xFF;
    for (size_t i = 0; i < len; i++) {
        crc ^= data[i];
        for (int b = 0; b < 8; b++)
            crc = (crc & 0x80) ? (crc << 1) ^ 0x31 : (crc << 1);
    }
    return crc;
}

static esp_err_t shtc3_write_cmd(uint16_t cmd) {
    i2c_cmd_handle_t h = i2c_cmd_link_create();
    i2c_master_start(h);
    i2c_master_write_byte(h, (SHTC3_ADDR << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(h, cmd >> 8, true);
    i2c_master_write_byte(h, cmd & 0xFF, true);
    i2c_master_stop(h);
    esp_err_t ret = i2c_master_cmd_begin(I2C_PORT, h, pdMS_TO_TICKS(20));
    i2c_cmd_link_delete(h);
    return ret;
}

static bool shtc3_read3(uint16_t meas_cmd, uint16_t *out) {
    uint8_t buf[3];
    i2c_cmd_handle_t h = i2c_cmd_link_create();
    i2c_master_start(h);
    i2c_master_write_byte(h, (SHTC3_ADDR << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(h, meas_cmd >> 8, true);
    i2c_master_write_byte(h, meas_cmd & 0xFF, true);
    i2c_master_start(h);  // repeated start
    i2c_master_write_byte(h, (SHTC3_ADDR << 1) | I2C_MASTER_READ, true);
    i2c_master_read(h, buf, 3, I2C_MASTER_LAST_NACK);
    i2c_master_stop(h);
    esp_err_t ret = i2c_master_cmd_begin(I2C_PORT, h, pdMS_TO_TICKS(20));
    i2c_cmd_link_delete(h);
    if (ret != ESP_OK || shtc3_crc(buf, 2) != buf[2]) return false;
    *out = ((uint16_t)buf[0] << 8) | buf[1];
    return true;
}

static bool shtc3_read(SensorData *out) {
    if (shtc3_write_cmd(SHTC3_CMD_WAKEUP) != ESP_OK) return false;
    vTaskDelay(pdMS_TO_TICKS(1));

    uint16_t raw_t, raw_rh;
    bool ok = shtc3_read3(SHTC3_CMD_MEAS_T,  &raw_t) &&
              shtc3_read3(SHTC3_CMD_MEAS_RH, &raw_rh);

    shtc3_write_cmd(SHTC3_CMD_SLEEP);
    if (!ok) return false;

    out->temp_c   = -45.0f + 175.0f * ((float)raw_t  / 65535.0f);
    out->humidity = 100.0f           * ((float)raw_rh / 65535.0f);
    return true;
}

// ---------- Motor control ----------
void handleIncoming(Message& msg, const uint8_t* src_mac) {
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
    ledc_timer_config_t timer = {};
    timer.speed_mode      = LEDC_LOW_SPEED_MODE;
    timer.duty_resolution = LEDC_TIMER_12_BIT;
    timer.timer_num       = LEDC_TIMER_0;
    timer.freq_hz         = 1000;
    timer.clk_cfg         = LEDC_AUTO_CLK;
    ledc_timer_config(&timer);

    ledc_channel_config_t chan = {};
    chan.gpio_num   = ENA_PWM_PIN;
    chan.speed_mode = LEDC_LOW_SPEED_MODE;
    chan.channel    = LEDC_CHANNEL_0;
    chan.timer_sel  = LEDC_TIMER_0;
    chan.duty       = 0;
    ledc_channel_config(&chan);

    // 3. Setup I2C for SHTC3
    i2c_config_t i2c_cfg = {};
    i2c_cfg.mode             = I2C_MODE_MASTER;
    i2c_cfg.sda_io_num       = I2C_SDA_IO;
    i2c_cfg.scl_io_num       = I2C_SCL_IO;
    i2c_cfg.sda_pullup_en    = GPIO_PULLUP_ENABLE;
    i2c_cfg.scl_pullup_en    = GPIO_PULLUP_ENABLE;
    i2c_cfg.master.clk_speed = I2C_FREQ_HZ;
    ESP_ERROR_CHECK(i2c_param_config(I2C_PORT, &i2c_cfg));
    ESP_ERROR_CHECK(i2c_driver_install(I2C_PORT, I2C_MODE_MASTER, 0, 0, 0));

    // 4. Start ESP-NOW, add remote sender as peer
    espNow.begin();
    espNow.addPeer(remote_mac);
    espNow.onReceive(handleIncoming);

    // 5. Periodically read SHTC3 and send data back to remote
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(2000));

        SensorData data;
        if (shtc3_read(&data)) {
            espNow.sendAny(remote_mac, data);
            printf("Sensor -> %.2f C  %.2f%%RH\n", data.temp_c, data.humidity);
        } else {
            printf("SHTC3 read failed\n");
        }
    }
}
