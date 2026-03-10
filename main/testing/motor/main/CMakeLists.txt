idf_component_register(SRCS "motor.c"
                    INCLUDE_DIRS "."
		    REQUIRES driver freertos
		    PRIV_REQUIRES esp_driver_gpio esp_driver_ledc esp_adc)
