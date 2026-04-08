idf_component_register(SRCS "display_module.c"
                       INCLUDE_DIRS "."
                       REQUIRES driver esp_lcd shared_data waveshare__esp_lcd_st7735 esp_lvgl_port lvgl)