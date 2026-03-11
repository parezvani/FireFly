idf_component_register(SRCS "scripts/RemoteSender.cpp" 
                            "scripts/MotorReceiver.cpp" 
                            "scripts/ESPNowEasy.cpp"
                    INCLUDE_DIRS "scripts"
                    REQUIRES esp_adc esp_wifi esp_now nvs_flash driver)