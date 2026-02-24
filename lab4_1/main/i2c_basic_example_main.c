/*
 * SPDX-FileCopyrightText: 2024 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Unlicense OR CC0-1.0
 */
/* i2c - Simple Example

   Simple I2C example that shows how to initialize I2C
   as well as reading and writing from and to registers for a sensor connected over I2C.

   The sensor used in this example is a MPU9250 inertial measurement unit.
*/
#include <stdio.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "driver/i2c_master.h"
#include "esp_system.h"
#include <math.h>

#define I2C_MASTER_SCL_IO           8                           /*!< GPIO number used for I2C master clock */
#define I2C_MASTER_SDA_IO           10                          /*!< GPIO number used for I2C master data  */
#define I2C_MASTER_NUM              I2C_NUM_0                   /*!< I2C port number for master dev */
#define I2C_MASTER_FREQ_HZ          40000                       /*!< I2C master clock frequency */
#define I2C_MASTER_TX_BUF_DISABLE   0                           /*!< I2C master doesn't need buffer */
#define I2C_MASTER_RX_BUF_DISABLE   0                           /*!< I2C master doesn't need buffer */
#define I2C_MASTER_TIMEOUT_MS       1000

#define ICM_ADDR 		    0x68
#define WHOAMI_ADDR		    0x75
#define PWR_MGMTO		    0x1F
#define PWR_WAKEUP		    0x03
#define PWR_SLEEP		    0x00
#define ACCEL_DATA_X1 		    0x0B

/**
 * @brief Read a sequence of bytes from a MPU9250 sensor registers
 */
static esp_err_t register_read(i2c_master_dev_handle_t dev_handle, uint8_t reg_addr, uint8_t *data, size_t len)
{
    return i2c_master_transmit_receive(dev_handle, &reg_addr, 1, data, len, I2C_MASTER_TIMEOUT_MS);
}

/**
 * @brief Write a byte to a MPU9250 sensor register
 */
static esp_err_t register_write_byte(i2c_master_dev_handle_t dev_handle, uint8_t reg_addr, uint8_t data)
{
    uint8_t write_buf[2] = {reg_addr, data};
    return i2c_master_transmit(dev_handle, write_buf, sizeof(write_buf), I2C_MASTER_TIMEOUT_MS);

}

/**
 * @brief i2c master initialization
 */
static void i2c_master_init(i2c_master_bus_handle_t *bus_handle, i2c_master_dev_handle_t *dev_handle)
{
    i2c_master_bus_config_t bus_config = {
        .i2c_port = I2C_MASTER_NUM,
        .sda_io_num = I2C_MASTER_SDA_IO,
        .scl_io_num = I2C_MASTER_SCL_IO,
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .glitch_ignore_cnt = 7,
        .flags.enable_internal_pullup = true,
    };
    ESP_ERROR_CHECK(i2c_new_master_bus(&bus_config, bus_handle));

    i2c_device_config_t dev_config = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = ICM_ADDR,
        .scl_speed_hz = I2C_MASTER_FREQ_HZ,
    };
    ESP_ERROR_CHECK(i2c_master_bus_add_device(*bus_handle, &dev_config, dev_handle));
}

void app_main(void)
{
    i2c_master_bus_handle_t bus_handle;
    i2c_master_dev_handle_t dev_handle;

    i2c_master_init(&bus_handle, &dev_handle);
    
    uint8_t who = 0;
    ESP_ERROR_CHECK(register_read(dev_handle, WHOAMI_ADDR, &who, 1));

    while(1) {
	uint8_t raw_inc[6];
        ESP_ERROR_CHECK(register_write_byte(dev_handle, PWR_MGMTO, PWR_WAKEUP));
        vTaskDelay(pdMS_TO_TICKS(10));
        ESP_ERROR_CHECK(register_read(dev_handle, ACCEL_DATA_X1, raw_inc, 6));
        ESP_ERROR_CHECK(register_write_byte(dev_handle, PWR_MGMTO, PWR_SLEEP));

        int16_t raw_x = (raw_inc[0] << 8) | raw_inc[1];
        int16_t raw_y = (raw_inc[2] << 8) | raw_inc[3];
        int16_t raw_z = (raw_inc[4] << 8) | raw_inc[5];

        float scale = 1.0f/16384.0f;   //16-bit signed integer to gram(g) unit
        float x = raw_x * scale;
        float y = raw_y * scale;
        float z = raw_z * scale;

        float left_right = atan2f(y, z) * 180.0f/M_PI;
        float up_down = atan2f(-x, sqrtf(y*y + z*z)) * 180.0f/M_PI;

        char dir[20] = "";
        if (up_down > 15.0f) strcat(dir, "Up ");
        else if (up_down < -15.0f) strcat(dir, "Down ");
        if (left_right > 15.0f) strcat(dir, "Right ");
        else if (left_right < -15.0f) strcat(dir, "Left ");

        ssize_t len = strlen(dir);
        if (len>0 && dir[len-1] == ' ') dir[len-1] = '\0';

        //printf("Pitch: %.1fdegree, Roll: %.1fdegree --> %s/n", up_down, left_right, dir);
        ESP_LOGI("", "%s", strlen(dir)>0? dir:"");
        vTaskDelay(pdMS_TO_TICKS(200));
    }
}
