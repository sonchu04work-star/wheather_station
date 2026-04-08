#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

#include "wheather_station.h"
#include "sensor_module.h"
#include "gps_module.h"
#include "display_module.h"
#include "network_module.h"


#define PYCHARM_SERVER_URL "http://172.20.10.11:5001/api/weather" 
#define WIFI_SSID "Binnn"
#define WIFI_PASS "nnnnnnnn"

static const char *TAG = "APP_MAIN";
weather_data_t current_data = {0};

void app_main(void)
{
    ESP_LOGI(TAG, "==== KHOI DONG TRAM THOI TIET DI DONG ====");

    // 1. Khởi tạo toàn bộ module
    display_module_init();
    sensor_module_init();
    gps_module_init();
    network_module_init(WIFI_SSID, WIFI_PASS);

    // Chờ một chút để WiFi cấp IP
    vTaskDelay(pdMS_TO_TICKS(5000));

    // 2. Vòng lặp thu thập và gửi dữ liệu
    while (1) {
        ESP_LOGI(TAG, "--- Dang lay du lieu moi ---");

        // Cập nhật dữ liệu vào struct dùng chung
        sensor_module_read(&current_data.temperature, &current_data.humidity);
        gps_module_get_latest(&current_data.latitude, &current_data.longitude, &current_data.gps_valid);

        // Xuất ra màn hình
        display_module_update(&current_data);

        // Đẩy lên Server
        network_module_send_data(PYCHARM_SERVER_URL, &current_data);

        // Lặp lại sau mỗi 10 giây
        vTaskDelay(pdMS_TO_TICKS(10000));
    }
}