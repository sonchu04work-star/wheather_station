#include "gps_module.h"
#include "driver/uart.h"
#include "esp_log.h"
#include "string.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define GPS_UART_PORT UART_NUM_1
#define GPS_RX_PIN 16
#define GPS_TX_PIN 17
#define BUF_SIZE 1024

static const char *TAG = "GPS_MOD";

// Biến nội bộ lưu trạng thái
static float current_lat = 0.0;
static float current_lon = 0.0;
static bool has_fix = false;

// Hàm phụ trợ: Chuyển đổi định dạng NMEA sang độ thập phân (Decimal Degrees)
static float convert_to_decimal(float nmea_coord) {
    int degrees = (int)(nmea_coord / 100);
    float minutes = nmea_coord - (degrees * 100);
    return degrees + (minutes / 60.0);
}

// Task chạy ngầm để liên tục đọc dữ liệu từ mạch NEO-6M
static void gps_task(void *arg) {
    uint8_t data[BUF_SIZE];
    while (1) {
        // Đọc dữ liệu từ cổng UART
        int len = uart_read_bytes(GPS_UART_PORT, data, BUF_SIZE - 1, 1000 / portTICK_PERIOD_MS);
        if (len > 0) {
            data[len] = '\0'; // Kết thúc chuỗi
            
            // Tìm câu lệnh $GPRMC chứa tọa độ
            char *rmc = strstr((char*)data, "$GPRMC");
            if (rmc) {
                char status;
                float lat_raw, lon_raw;
                char lat_dir, lon_dir;
                
                // Cắt chuỗi để lấy các thông số cần thiết
                int parsed = sscanf(rmc, "$GPRMC,%*f,%c,%f,%c,%f,%c", 
                                    &status, &lat_raw, &lat_dir, &lon_raw, &lon_dir);
                
                // Nếu đọc thành công và vệ tinh đã chốt (Status = 'A')
                if (parsed == 5 && status == 'A') {
                    has_fix = true;
                    
                    current_lat = convert_to_decimal(lat_raw);
                    if (lat_dir == 'S') current_lat = -current_lat; // Bán cầu Nam
                    
                    current_lon = convert_to_decimal(lon_raw);
                    if (lon_dir == 'W') current_lon = -current_lon; // Bán cầu Tây
                    
                    ESP_LOGD(TAG, "Da chot vi tri: %.5f, %.5f", current_lat, current_lon);
                } else {
                    has_fix = false;
                }
            }
        }
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

void gps_module_init(void) {
    // Cấu hình thông số UART cho NEO-6M
    uart_config_t uart_config = {
        .baud_rate = 9600,
        .data_bits = UART_DATA_8_BITS,
        .parity    = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE
    };
    
    uart_param_config(GPS_UART_PORT, &uart_config);
    uart_set_pin(GPS_UART_PORT, GPS_TX_PIN, GPS_RX_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    uart_driver_install(GPS_UART_PORT, BUF_SIZE * 2, 0, 0, NULL, 0);

    // Tạo luồng (Task) riêng để luôn luôn lắng nghe GPS mà không làm kẹt main
    xTaskCreate(gps_task, "gps_task", 4096, NULL, 5, NULL);
    
    ESP_LOGI(TAG, "Khoi tao GPS UART (Khong can NMEA Parser) thanh cong.");
}

void gps_module_get_latest(float *lat, float *lon, bool *is_valid) {
    *lat = current_lat;
    *lon = current_lon;
    *is_valid = has_fix;
}