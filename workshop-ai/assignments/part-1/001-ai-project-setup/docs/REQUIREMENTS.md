# Requirements for the ESP32-S3-EYE Board Bringup

## Hardware

The project must consider the following hardware.

| Component | Details |
|-----------|---------|
| Board | ESP32-S3-EYE |
| SoC | ESP32-S3 |
| Flash | 8 MB |
| PSRAM | 8 MB |
| Camera sensor | OV2640 (DVP interface) |
| Display | ST7789 240×240 px (SPI, backlight via BSP) |
| Power LED | Active-high, GPIO3 |

## Functional Requirements

- Display a live camera preview that fills the full 240×240 LCD at 25 fps.
- Camera output must be in RGB565 format — no software color conversion.
- The camera sensor must output natively at 240×240 — no software scaling.
- The module power LED (GPIO3) must be turned off at startup.
- The application must boot and run without errors from a cold start.
- All the hardware initialization must be according to the BSP for the ESP32-S3-EYE.
- All configurstions must be via KConfig entries.
- If applicable, hardware specific configurations must be set on the `sdkconfig.defaults`.

## Software Requirements

| Item | Version / Value |
|------|----------------|
| ESP-IDF | v5.5.4 |
| BSP component | `espressif/esp32_s3_eye ^6.0.0` |
| Camera driver | `espressif/esp_video` (transitive via BSP) |
| Display / LVGL | `espressif/esp_lvgl_port` + LVGL 9.x (transitive via BSP) |
| IDF target | `esp32s3` |
