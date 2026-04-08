#include "display_module.h"
#include "driver/spi_master.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_vendor.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_st7735.h"
#include "esp_log.h"

// --- THÊM THƯ VIỆN LVGL ---
#include "lvgl.h"
#include "esp_lvgl_port.h"

#define PIN_NUM_MISO -1
#define PIN_NUM_MOSI 23
#define PIN_NUM_CLK  18
#define PIN_NUM_CS   5
#define PIN_NUM_DC   19
#define PIN_NUM_RST  2
#define PIN_NUM_BCKL 4

static const char *TAG = "LCD_LVGL";
static esp_lcd_panel_handle_t panel_handle = NULL;

// Các biến lưu trữ đối tượng chữ trên màn hình
static lv_obj_t *label_temp;
static lv_obj_t *label_hum;
static lv_obj_t *label_gps;

void display_module_init(void) {
    // 1. Khởi tạo SPI và LCD
    spi_bus_config_t buscfg = {
        .sclk_io_num = PIN_NUM_CLK, .mosi_io_num = PIN_NUM_MOSI,
        .miso_io_num = PIN_NUM_MISO, .quadwp_io_num = -1, .quadhd_io_num = -1,
        .max_transfer_sz = 128 * 160 * 2 + 8
    };
    spi_bus_initialize(SPI2_HOST, &buscfg, SPI_DMA_CH_AUTO);

    esp_lcd_panel_io_handle_t io_handle = NULL;
    esp_lcd_panel_io_spi_config_t io_config = {
        .dc_gpio_num = PIN_NUM_DC, .cs_gpio_num = PIN_NUM_CS,
        .pclk_hz = 20 * 1000 * 1000, .lcd_cmd_bits = 8, .lcd_param_bits = 8,
        .spi_mode = 0, .trans_queue_depth = 10,
    };
    esp_lcd_new_panel_io_spi((esp_lcd_spi_bus_handle_t)SPI2_HOST, &io_config, &io_handle);

    esp_lcd_panel_dev_config_t panel_config = {
        .reset_gpio_num = PIN_NUM_RST,
        .color_space = ESP_LCD_COLOR_SPACE_RGB,
        .bits_per_pixel = 16,
    };
    esp_lcd_new_panel_st7735(io_handle, &panel_config, &panel_handle);
    esp_lcd_panel_reset(panel_handle);
    esp_lcd_panel_init(panel_handle);
    esp_lcd_panel_disp_on_off(panel_handle, true);

    // 2. Cấu hình LVGL (CHẾ ĐỘ TIẾT KIỆM RAM)
    const lvgl_port_cfg_t lvgl_cfg = ESP_LVGL_PORT_INIT_CONFIG();
    lvgl_port_init(&lvgl_cfg);

    const lvgl_port_display_cfg_t disp_cfg = {
        .io_handle = io_handle,
        .panel_handle = panel_handle,
        .buffer_size = 128 * 20,        // <--- RAM thấp (Chỉ load 1/8 màn hình)
        .double_buffer = false,
        .hres = 128, .vres = 160,
        .monochrome = false,
        .flags = { .buff_dma = true, .swap_bytes = true }
    };
    lv_disp_t * disp = lvgl_port_add_disp(&disp_cfg);

    // 3. VẼ GIAO DIỆN CÓ MÀU SẮC
    lvgl_port_lock(0);
    
    // Nền đen
    lv_obj_t *scr = lv_disp_get_scr_act(disp);
    lv_obj_set_style_bg_color(scr, lv_color_hex(0x000000), LV_PART_MAIN);

    // Tiêu đề (Màu Cyan)
    lv_obj_t *title = lv_label_create(scr);
    lv_label_set_text(title, "TRAM THOI TIET");
    lv_obj_set_style_text_color(title, lv_color_hex(0x00FFFF), LV_PART_MAIN); 
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 10);

    // Label Nhiệt độ (Màu Cam)
    label_temp = lv_label_create(scr);
    lv_label_set_text(label_temp, "Nhiet do: --.- C");
    lv_obj_set_style_text_color(label_temp, lv_color_hex(0xFFA500), LV_PART_MAIN);
    lv_obj_align(label_temp, LV_ALIGN_TOP_LEFT, 5, 50);

    // Label Độ ẩm (Màu Xanh lá)
    label_hum = lv_label_create(scr);
    lv_label_set_text(label_hum, "Do am: --.- %");
    lv_obj_set_style_text_color(label_hum, lv_color_hex(0x00FF00), LV_PART_MAIN);
    lv_obj_align(label_hum, LV_ALIGN_TOP_LEFT, 5, 80);

    // Label GPS (Màu Trắng lúc mới bật)
    label_gps = lv_label_create(scr);
    lv_label_set_text(label_gps, "GPS: Searching...");
    lv_obj_set_style_text_color(label_gps, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_align(label_gps, LV_ALIGN_TOP_LEFT, 5, 110);

    lvgl_port_unlock(); 
    ESP_LOGI(TAG, "Khoi tao giao dien LVGL (Co Mau, Low RAM) thanh cong!");
}

void display_module_update(weather_data_t *data) {
    if (lvgl_port_lock(0)) {
        // --- CÁCH KHẮC PHỤC LỖI CHỮ 'f' ---
        // 1. Tạo 2 bộ đệm (buffer) chuẩn C để chứa chữ
        char temp_str[32];
        char hum_str[32];
        
        // 2. Dùng hàm C chuẩn để đóng gói số thực vào chuỗi
        snprintf(temp_str, sizeof(temp_str), "Nhiet do: %.1f C", data->temperature);
        snprintf(hum_str, sizeof(hum_str), "Do am: %.1f %%", data->humidity);
        
        // 3. Đưa chuỗi đã đóng gói cho LVGL in ra
        lv_label_set_text(label_temp, temp_str);
        lv_label_set_text(label_hum, hum_str);
        // -----------------------------------
        
        // Hiệu ứng đổi màu cảnh báo GPS
        if (data->gps_valid) {
            lv_label_set_text(label_gps, "GPS: Fixed");
            lv_obj_set_style_text_color(label_gps, lv_color_hex(0x00FF00), LV_PART_MAIN); // Xanh lá khi có sóng
        } else {
            lv_label_set_text(label_gps, "GPS: Searching...");
            lv_obj_set_style_text_color(label_gps, lv_color_hex(0xFF0000), LV_PART_MAIN); // Đỏ khi mất sóng
        }
        
        lvgl_port_unlock();
    }
}