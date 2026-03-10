#include "ESPNowEasy.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

struct Message {
    int counter;
    char text[32];
};

ESPNowEasy<Message> espNow;
Message dataToSend;

extern "C" void app_main(void)
{
    if (!espNow.begin()) {
        ESP_LOGE(TAG, "ESP-NOW init failed");
        return;
    }

    dataToSend = {0, "Hello World!"};

    while (1) {
        espNow.send(dataToSend);
        dataToSend.counter++;

        ESP_LOGI(TAG, "Sent message %d", dataToSend.counter);

        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}