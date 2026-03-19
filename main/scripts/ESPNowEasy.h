#ifndef ESPNOWEASY_H
#define ESPNOWEASY_H

#include "esp_now.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "esp_log.h"
#include "nvs_flash.h"

template<typename MessageType>
class ESPNowEasy {
public:
    typedef void (*ReceiverCallback)(MessageType& message, const uint8_t* src_mac);

    bool begin(const uint8_t* peerAddr = nullptr);
    bool send(MessageType& message);
    bool addPeer(const uint8_t* peerAddr);
    template<typename T> bool sendAny(const uint8_t* addr, T& message);
    void onReceive(ReceiverCallback callback);

private:
    uint8_t targetAddress[6];
    static ReceiverCallback userCallback;
    static void receiveCallback(const esp_now_recv_info_t *info, const uint8_t *data, int len);
};

// Keep this include here for Templates to work correctly
#include "ESPNowEasy.cpp"

#endif
