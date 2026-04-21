# ESP32 Weather Station & GPS Tracker (Full-stack IoT System)

![ESP32](https://img.shields.io/badge/Hardware-ESP32-blue)
![ESP-IDF](https://img.shields.io/badge/Framework-ESP--IDF-red)
![LVGL](https://img.shields.io/badge/UI-LVGL-green)
![Python](https://img.shields.io/badge/Backend-Python%20Flask-yellow)
![MySQL](https://img.shields.io/badge/Database-MySQL-blue)

Một hệ thống trạm thời tiết và định vị GPS IoT hoàn chỉnh. Dự án bao gồm việc thu thập dữ liệu từ cảm biến (Nhiệt độ, Độ ẩm) và tọa độ (GPS NEO-6M), hiển thị trực quan lên màn hình TFT thông qua hệ điều hành giao diện **LVGL** (được tối ưu hóa bộ nhớ), và gửi dữ liệu theo thời gian thực về một máy chủ **Python Flask**. Máy chủ sẽ tự động đánh giá trạng thái thời tiết và lưu trữ vào cơ sở dữ liệu **MySQL**.

## Các tính năng chính

- **Hardware (C/C++ & ESP-IDF):**
  - Đọc dữ liệu môi trường (Nhiệt độ, Độ ẩm).
  - Trích xuất tọa độ kinh độ/vĩ độ cực kỳ chính xác từ module GPS (Hỗ trợ cả chuẩn `$GPRMC` và `$GNRMC`).
  - Giao diện người dùng trực quan trên màn hình TFT ST7735 sử dụng **LVGL** (Đã tối ưu hóa RAM, chạy mượt mà không cần PSRAM).
  - Tự động đóng gói dữ liệu thành JSON và gửi qua giao thức HTTP POST.

- **Backend (Python & Flask):**
  - Tự động dò tìm và hiển thị địa chỉ IP LAN để dễ dàng cấu hình cho ESP32.
  - Xử lý API RESTful (Nhận dữ liệu từ mạch và Cung cấp dữ liệu cho Frontend).
  - Tích hợp logic đánh giá thời tiết thông minh dựa trên nhiệt độ (Nóng bức, Nóng, Mát mẻ, Lạnh).
  - Lưu trữ dữ liệu an toàn vào MySQL.

---

## Cấu trúc phần cứng (Wiring Diagram)

Dự án sử dụng vi điều khiển ESP32, màn hình 1.8" TFT SPI (ST7735) và mạch định vị GPS NEO-6M.

### 1. Màn hình TFT SPI (ST7735)
| Chân Màn Hình | Chức năng (SPI) | Chân ESP32 |
| :--- | :--- | :--- |
| **VCC** | Nguồn | 3V3 / 5V |
| **GND** | Nối đất | GND |
| **CS** | Chip Select | GPIO 5 |
| **RESET** | Reset | GPIO 2 |
| **A0 (DC)** | Data / Command | GPIO 19 |
| **SDA (MOSI)**| Master Out Slave In | GPIO 23 |
| **SCK** | Clock | GPIO 18 |
| **LED** | Đèn nền (Backlight)| GPIO 4 (hoặc 3V3) |

### 2. Module GPS NEO-6M
| Chân GPS | Chân ESP32 (UART1) | Ghi chú |
| :--- | :--- | :--- |
| VCC | 3V3 / 5V | Nguồn cấp |
| GND | GND | Nối đất |
| **TX** | **RX (GPIO 16)** | *Cắm chéo* |
| **RX** | **TX (GPIO 17)** | *Cắm chéo* |

---

## Hướng dẫn Cài đặt & Chạy dự án

### Phần 1: Thiết lập Máy chủ Database (MySQL)
Cài đặt MySQL Server trên máy tính.

Tạo database và bảng dữ liệu bằng lệnh SQL sau:

SQL
CREATE DATABASE weather_station_db;
USE weather_station_db;

CREATE TABLE sensor_data (
    id INT AUTO_INCREMENT PRIMARY KEY,
    temperature FLOAT,
    humidity FLOAT,
    weather_status VARCHAR(50),
    latitude FLOAT,
    longitude FLOAT,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);
### Phần 2: Khởi động Python Server
Cài đặt các thư viện Python cần thiết:

Bash
pip install flask flask-cors mysql-connector-python
Cấu hình lại tài khoản/mật khẩu MySQL trong file app.py cho khớp với máy của bạn.

Chạy server:

Bash
python app.py
Lưu ý: Sau khi chạy, Terminal của Python sẽ tự động in ra một đường link chứa địa chỉ IP mạng LAN (VD: http://192.168.1.15:5001/api/weather). Hãy copy đường link này.

### Phần 3: Nạp Code cho mạch ESP32
Mở dự án wheather_station bằng VS Code (đã cài extension ESP-IDF).

Dán đường link IP vừa copy ở bước trên vào biến PYCHARM_SERVER_URL trong file main/wheather_station.c.

Thay đổi thông tin WiFi (WIFI_SSID và WIFI_PASS) thành mạng của bạn.

Mở file sdkconfig (nằm ở thư mục gốc dự án) và đảm bảo các dòng sau được cấu hình để tối ưu bộ nhớ cho LVGL:

Ini, TOML
CONFIG_LV_ANTIALIAS=n
CONFIG_LV_USE_ANIMATION=n
CONFIG_LV_USE_SHADOW=n
CONFIG_PARTITION_TABLE_SINGLE_APP=n
CONFIG_PARTITION_TABLE_SINGLE_APP_LARGE=y
Dọn dẹp, Build và Nạp code:

Bash
idf.py fullclean
idf.py build flash monitor
Tài liệu API (API Endpoints)
Máy chủ Python cung cấp 2 API chính:

1. POST /api/weather
Mô tả: Nhận dữ liệu từ mạch ESP32 và lưu vào Database.

Payload (JSON Body):

JSON
{
    "temperature": 29.5,
    "humidity": 65.0,
    "lat": 10.762622,
    "lon": 106.660172,
    "gps_valid": true
}
2. GET /api/latest-data
Mô tả: Frontend/Web app gọi API này để lấy dòng dữ liệu thời tiết mới nhất.

Response (JSON):

JSON
{
    "id": 102,
    "temperature": 29.5,
    "humidity": 65.0,
    "weather_status": "Nóng",
    "latitude": 10.762622,
    "longitude": 106.660172,
    "created_at": "2024-05-15 14:30:00"
}
