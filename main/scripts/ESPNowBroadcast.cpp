#include "ESPNowEasy.h"

struct Message {
    int counter;
    char text[32];
};

// TERMINAL A's MAC ADDRESS
uint8_t drone_mac[] = {0x64, 0xE8, 0x33, 0xDC, 0xA2, 0xDC};

ESPNowEasy<Message> espNow;

extern "C" void app_main(void) {
    espNow.begin(drone_mac); // Pass the address here
    Message data = {0, "Drone Command"};

    while (1) {
        espNow.send(data);
        printf("Sent: %d\n", data.counter++);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
