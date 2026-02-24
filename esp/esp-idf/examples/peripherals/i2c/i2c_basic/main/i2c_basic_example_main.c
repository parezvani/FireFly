
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/i2c.h"
#include "esp_system.h"
#include "esp_log.h"

#define I2C_MASTER_SCL_IO         8        /*!< GPIO number for I2C master clock */
#define I2C_MASTER_SDA_IO         10       /*!< GPIO number for I2C master data  */
#define I2C_MASTER_NUM            I2C_NUM_0
#define I2C_MASTER_FREQ_HZ        100000
#define I2C_MASTER_TX_BUF_DISABLE 0
#define I2C_MASTER_RX_BUF_DISABLE 0

#define SHTC3_SENSOR_ADDR   0x70
#define SHTC3_CMD_SLEEP     0xB098
#define SHTC3_CMD_WAKEUP    0x3517

#define SHTC3_CMD_MEAS_T_FIRST   0x7CA2   /* Read temperature first */
#define SHTC3_CMD_MEAS_RH_FIRST  0x5C24   /* Read humidity first   */

static const char *TAG = "SHTC3_APP";

static esp_err_t i2c_master_init(void)
{
    i2c_config_t cfg = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = I2C_MASTER_SDA_IO,
        .scl_io_num = I2C_MASTER_SCL_IO,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = I2C_MASTER_FREQ_HZ,
    };
    ESP_ERROR_CHECK(i2c_param_config(I2C_MASTER_NUM, &cfg));
    return i2c_driver_install(I2C_MASTER_NUM, cfg.mode,
                              I2C_MASTER_TX_BUF_DISABLE,
                              I2C_MASTER_RX_BUF_DISABLE, 0);
}

static uint8_t shtc3_crc(uint8_t *data, size_t len)
{
    uint8_t crc = 0xFF;               /* initial value */
    for (size_t i = 0; i < len; ++i) {
        crc ^= data[i];
        for (uint8_t b = 0; b < 8; ++b) {
            crc = (crc & 0x80) ? (crc << 1) ^ 0x31 : (crc << 1);
        }
    }
    return crc;
}

static esp_err_t shtc3_power_up(void)
{
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (SHTC3_SENSOR_ADDR << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, SHTC3_CMD_WAKEUP >> 8, true);
    i2c_master_write_byte(cmd, SHTC3_CMD_WAKEUP & 0xFF, true);
    i2c_master_stop(cmd);

    esp_err_t ret = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, pdMS_TO_TICKS(20));
    i2c_cmd_link_delete(cmd);

    /* Datasheet: max 240 µs until sensor is ready. We wait 1 ms for safety. */
    vTaskDelay(pdMS_TO_TICKS(1));
    return ret;
}

static esp_err_t shtc3_power_down(void)
{
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (SHTC3_SENSOR_ADDR << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, SHTC3_CMD_SLEEP >> 8, true);
    i2c_master_write_byte(cmd, SHTC3_CMD_SLEEP & 0xFF, true);
    i2c_master_stop(cmd);

    esp_err_t ret = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, pdMS_TO_TICKS(20));
    i2c_cmd_link_delete(cmd);
    return ret;
}


/* Convert raw 16‑bit values to physical units (datasheet formulas) */
static inline float raw_to_celsius(uint16_t raw)
{
    return -45.0f + 175.0f * ((float)raw / 65535.0f);
}
static inline float raw_to_rh(uint16_t raw)
{
    return 100.0f * ((float)raw / 65535.0f);
}


//  * Each function reads *exactly three bytes* (2 bytes data + 1 byte CRC) as required.
static esp_err_t shtc3_read_temperature(float *temp_c)
{
    uint8_t buf[3];
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();

    /* Send measurement command (Temperature‑first) */
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (SHTC3_SENSOR_ADDR << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, SHTC3_CMD_MEAS_T_FIRST >> 8, true);
    i2c_master_write_byte(cmd, SHTC3_CMD_MEAS_T_FIRST & 0xFF, true);

    /* Repeated start – read three bytes only */
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (SHTC3_SENSOR_ADDR << 1) | I2C_MASTER_READ, true);
    i2c_master_read(cmd, buf, sizeof(buf), I2C_MASTER_LAST_NACK);
    i2c_master_stop(cmd);

    esp_err_t ret = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, pdMS_TO_TICKS(20));
    i2c_cmd_link_delete(cmd);
    if (ret != ESP_OK) {
        return ret;
    }

    /* CRC check */
    if (shtc3_crc(buf, 2) != buf[2]) {
        ESP_LOGE(TAG, "Temperature CRC mismatch");
        return ESP_ERR_INVALID_CRC;
    }

    uint16_t raw = ((uint16_t)buf[0] << 8) | buf[1];
    *temp_c = raw_to_celsius(raw);
    return ESP_OK;
}

static esp_err_t shtc3_read_humidity(float *rh)
{
    uint8_t buf[3];
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();

    /* Send measurement command (Humidity‑first) */
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (SHTC3_SENSOR_ADDR << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, SHTC3_CMD_MEAS_RH_FIRST >> 8, true);
    i2c_master_write_byte(cmd, SHTC3_CMD_MEAS_RH_FIRST & 0xFF, true);

    /* Repeated start – read three bytes only */
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (SHTC3_SENSOR_ADDR << 1) | I2C_MASTER_READ, true);
    i2c_master_read(cmd, buf, sizeof(buf), I2C_MASTER_LAST_NACK);
    i2c_master_stop(cmd);

    esp_err_t ret = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, pdMS_TO_TICKS(20));
    i2c_cmd_link_delete(cmd);
    if (ret != ESP_OK) {
        return ret;
    }

    /* CRC check */
    if (shtc3_crc(buf, 2) != buf[2]) {
        ESP_LOGE(TAG, "Humidity CRC mismatch");
        return ESP_ERR_INVALID_CRC;
    }

    uint16_t raw = ((uint16_t)buf[0] << 8) | buf[1];
    *rh = raw_to_rh(raw);
    return ESP_OK;
}


 
static inline float celsius_to_fahrenheit(float c)
{
    return c * 1.8f + 32.0f;
}

void app_main(void)
{
    if (i2c_master_init() != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialise I2C");
        return;
    }

    /* Ensure power‑up is executed at most once every 2 s */
    const TickType_t cycle_delay = pdMS_TO_TICKS(2000);

    for (;;) {
        /* Power‑up sensor */
        if (shtc3_power_up() != ESP_OK) {
            ESP_LOGE(TAG, "Sensor wake‑up failed");
            vTaskDelay(cycle_delay);
            continue;
        }

        float temp_c, rh;
        esp_err_t t_ret = shtc3_read_temperature(&temp_c);
        esp_err_t h_ret = shtc3_read_humidity(&rh);

        /* Always put the sensor back to sleep regardless of read outcome */
        shtc3_power_down();

        if (t_ret == ESP_OK && h_ret == ESP_OK) {
            float temp_f = celsius_to_fahrenheit(temp_c);
            ESP_LOGI(TAG, "Temperature: %.2f °C (%.2f °F), Humidity: %.2f %%RH", temp_c, temp_f, rh);
        } else {
            ESP_LOGE(TAG, "Read error (T:%d, RH:%d)", t_ret, h_ret);
        }

        /* Wait ≥2 s before the next power‑up to respect the requirement */
        vTaskDelay(cycle_delay);
    }
}
