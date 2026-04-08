idf_component_register(SRCS "network_module.c"
                       INCLUDE_DIRS "."
                       REQUIRES esp_wifi nvs_flash esp_netif esp_event esp_http_client shared_data)