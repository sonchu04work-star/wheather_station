#include "sensor_module.h"
#include "driver/i2c.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define I2C_MASTER_SCL_IO           22      
#define I2C_MASTER_SDA_IO           21      
#define I2C_MASTER_NUM              I2C_NUM_0
#define AHT11_SENSOR_ADDR           0x38    

static const char *TAG = "SENSOR_AHT";

void sensor_module_init(void) {
    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = I2C_MASTER_SDA_IO,
        .scl_io_num = I2C_MASTER_SCL_IO,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = 100000,
    };
    i2c_param_config(I2C_MASTER_NUM, &conf);
    i2c_driver_install(I2C_MASTER_NUM, conf.mode, 0, 0, 0);
    ESP_LOGI(TAG, "Khoi tao I2C cho AHT11 thanh cong.");
}

void sensor_module_read(float *temperature, float *humidity) {
    uint8_t cmd[3] = {0xAC, 0x33, 0x00}; // Lệnh bắt đầu đo
    uint8_t data[6];

    // Gửi lệnh đo
    i2c_master_write_to_device(I2C_MASTER_NUM, AHT11_SENSOR_ADDR, cmd, sizeof(cmd), 1000 / portTICK_PERIOD_MS);
    vTaskDelay(pdMS_TO_TICKS(80)); // Chờ AHT11 xử lý

    // Đọc kết quả
    esp_err_t ret = i2c_master_read_from_device(I2C_MASTER_NUM, AHT11_SENSOR_ADDR, data, sizeof(data), 1000 / portTICK_PERIOD_MS);
    
    if (ret == ESP_OK && (data[0] & 0x80) == 0) { // Kiểm tra bit bận
        uint32_t hum_raw = ((uint32_t)data[1] << 12) | ((uint32_t)data[2] << 4) | (data[3] >> 4);
        uint32_t temp_raw = (((uint32_t)data[3] & 0x0F) << 16) | ((uint32_t)data[4] << 8) | data[5];

        *humidity = ((float)hum_raw / 1048576.0) * 100.0;
        *temperature = ((float)temp_raw / 1048576.0) * 200.0 - 50.0;
        ESP_LOGI(TAG, "Temp: %.2f C, Hum: %.2f %%", *temperature, *humidity);
    } else {
        ESP_LOGE(TAG, "Loi doc cam bien AHT11");
        *temperature = 0.0;
        *humidity = 0.0;
    }
}