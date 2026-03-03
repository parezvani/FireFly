#include "driver/spi_master.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_system.h"
#include "icm42670.h"
#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "soc/gpio_num.h"

#include <math.h>

static const char *TAG = "ICM42670_SPI_demo";
static icm42670_handle_t icm42670 = NULL;
static spi_device_handle_t spi_handle = NULL;

// SPI pins for ESP32-C3
#define PIN_NUM_MISO   GPIO_NUM_4
#define PIN_NUM_MOSI   GPIO_NUM_6
#define PIN_NUM_SCLK   GPIO_NUM_5
#define PIN_NUM_CS     GPIO_NUM_7

static void spi_bus_init(void)
{
    spi_bus_config_t buscfg = {
        .mosi_io_num = PIN_NUM_MOSI,
        .miso_io_num = PIN_NUM_MISO,
        .sclk_io_num = PIN_NUM_SCLK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 64,
    };
    ESP_ERROR_CHECK(spi_bus_initialize(SPI2_HOST, &buscfg, SPI_DMA_CH_AUTO));

    spi_device_interface_config_t devcfg = {
        .clock_speed_hz = 8 * 1000 * 1000,   // 8 MHz
        .mode = 0,                           // SPI mode 0
        .spics_io_num = PIN_NUM_CS,
        .queue_size = 3,
        .flags = SPI_DEVICE_HALFDUPLEX,
    };

    ESP_ERROR_CHECK(spi_bus_add_device(SPI2_HOST, &devcfg, &spi_handle));
}

static void imu_init(void)
{
    ESP_ERROR_CHECK(icm42670_create_spi(spi_handle, &icm42670));
    const icm42670_cfg_t cfg = {
        .acce_fs = ACCE_FS_2G,
        .acce_odr = ACCE_ODR_400HZ,
        .gyro_fs = GYRO_FS_2000DPS,
        .gyro_odr = GYRO_ODR_400HZ,
    };
    ESP_ERROR_CHECK(icm42670_config(icm42670, &cfg));
}

void app_main(void)
{
    icm42670_value_t acc = {0};
    icm42670_value_t gyro = {0};
    complimentary_angle_t angle = {0};

    spi_bus_init();
    imu_init();
    ESP_ERROR_CHECK(icm42670_acce_set_pwr(icm42670, ACCE_PWR_LOWNOISE));
    ESP_ERROR_CHECK(icm42670_gyro_set_pwr(icm42670, GYRO_PWR_LOWNOISE));

    while (1) {
        ESP_ERROR_CHECK(icm42670_get_acce_value(icm42670, &acc));
        ESP_ERROR_CHECK(icm42670_get_gyro_value(icm42670, &gyro));

        ESP_ERROR_CHECK(icm42670_complimentory_filter(icm42670, &acc, &gyro, &angle));

        ESP_LOGI(TAG, "ACCE X=%.3f Y=%.3f Z=%.3f", acc.x, acc.y, acc.z);
        ESP_LOGI(TAG, "GYRO X=%.3f Y=%.3f Z=%.3f", gyro.x, gyro.y, gyro.z);

        vTaskDelay(pdMS_TO_TICKS(100));
    }
}