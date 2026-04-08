#include "network_module.h"
#include "esp_wifi.h"
#include "nvs_flash.h"
#include "esp_http_client.h"
#include "esp_log.h"
#include <string.h>

static const char *TAG = "NET_MOD";

void network_module_init(const char* ssid, const char* pass) {
    nvs_flash_init();
    esp_netif_init();
    esp_event_loop_create_default();
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&cfg);

    wifi_config_t wifi_config = {0};
    strcpy((char*)wifi_config.sta.ssid, ssid);
    strcpy((char*)wifi_config.sta.password, pass);

    esp_wifi_set_mode(WIFI_MODE_STA);
    esp_wifi_set_config(WIFI_IF_STA, &wifi_config);
    esp_wifi_start();
    esp_wifi_connect();
    ESP_LOGI(TAG, "Da ra lenh ket noi WiFi: %s", ssid);
}

void network_module_send_data(const char* server_url, weather_data_t *data) {
    char post_data[200];
    // Đóng gói JSON
    snprintf(post_data, sizeof(post_data), 
             "{\"temperature\":%.2f,\"humidity\":%.2f,\"lat\":%.6f,\"lon\":%.6f,\"gps_valid\":%s}",
             data->temperature, data->humidity, data->latitude, data->longitude, 
             data->gps_valid ? "true" : "false");

    esp_http_client_config_t config = {
        .url = server_url,
        .method = HTTP_METHOD_POST,
    };
    esp_http_client_handle_t client = esp_http_client_init(&config);
    
    esp_http_client_set_header(client, "Content-Type", "application/json");
    esp_http_client_set_post_field(client, post_data, strlen(post_data));

    esp_err_t err = esp_http_client_perform(client);
    if (err == ESP_OK) {
        ESP_LOGI(TAG, "Gui Data Server PyCharm OK. Status = %d", esp_http_client_get_status_code(client));
    } else {
        ESP_LOGE(TAG, "Loi gui Data HTTP POST: %s", esp_err_to_name(err));
    }
    esp_http_client_cleanup(client);
}