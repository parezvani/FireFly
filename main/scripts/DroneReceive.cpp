#include "ESPNowEasy.h"
#include "esp_log.h"

struct Message {
    int counter;
    char text[32];
};

ESPNowEasy<Message> espNow;

// This is the "Ear" of the drone. It prints whatever the Remote sends.
void handleIncoming(Message& msg) {
    printf("--- RECEIVED DATA ---\n");
    printf("Counter: %d\n", msg.counter);
    printf("Text:    %s\n", msg.text);
    printf("---------------------\n");
}

extern "C" void app_main(void) {
    // 1. Initialize ESP-NOW (No MAC needed for receiver)
    espNow.begin(); 
    
    // 2. Tell the code to run 'handleIncoming' when data arrives
    espNow.onReceive(handleIncoming); 

    while(1) {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}