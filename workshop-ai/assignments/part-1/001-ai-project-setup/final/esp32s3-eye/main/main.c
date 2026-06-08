#include <stdio.h>
#include <string.h>

#include "sdkconfig.h"
#include "bsp/esp-bsp.h"
#include "driver/gpio.h"
#include "esp_err.h"
#include "esp_heap_caps.h"
#include "esp_log.h"
#include "esp_private/esp_cache_private.h"

#include "app_video.h"

#define NUM_BUFS 2
#define ALIGN_UP(num, align)    (((num) + ((align) - 1)) & ~((align) - 1))

/* Module Power LED is active-high on GPIO3; drive it low to turn it off */
#define MODULE_POWER_LED_GPIO   GPIO_NUM_3

static const char *TAG = "example";
static size_t data_cache_line_size = 0;
static lv_obj_t *camera_canvas = NULL;
static uint8_t *cam_buff[NUM_BUFS];
static uint32_t cam_buff_size = 0;

static void camera_video_frame_operation(uint8_t *camera_buf, uint8_t camera_buf_index, uint32_t camera_buf_hes,
        uint32_t camera_buf_ves, size_t camera_buf_len)
{
    uint32_t out_w = camera_buf_hes;
    uint32_t out_h = camera_buf_ves;
    uint8_t *out_buf = camera_buf;

    bsp_display_lock(0);
    lv_canvas_set_buffer(camera_canvas, out_buf, out_w, out_h, LV_COLOR_FORMAT_RGB565);
    lv_obj_center(camera_canvas);
    lv_obj_invalidate(camera_canvas);
    bsp_display_unlock();
}

void app_main(void)
{
    esp_err_t ret = ESP_OK;

    /* Turn off the module power LED (active-low, GPIO3) */
    gpio_config_t led_cfg = {
        .pin_bit_mask = BIT64(MODULE_POWER_LED_GPIO),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    gpio_config(&led_cfg);
    gpio_set_level(MODULE_POWER_LED_GPIO, 0);

    bsp_display_start();
    bsp_display_backlight_on();

    /* Initialize Camera */
    bsp_camera_start(NULL);

    /* Get cache line alignment for PSRAM allocations */
    ret = esp_cache_get_alignment(MALLOC_CAP_SPIRAM, &data_cache_line_size);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Get cache alignment failed: 0x%x", ret);
        return;
    }

    /* Allocate LVGL canvas buffers in PSRAM */
    cam_buff_size = ALIGN_UP(BSP_LCD_H_RES * BSP_LCD_V_RES * 2, data_cache_line_size);
    for (int i = 0; i < NUM_BUFS; i++) {
        cam_buff[i] = heap_caps_aligned_calloc(data_cache_line_size, 1, cam_buff_size, MALLOC_CAP_SPIRAM);
        if (cam_buff[i] == NULL) {
            ESP_LOGE(TAG, "Failed to allocate camera buffer %d", i);
            return;
        }
    }

    /* Create LVGL canvas for camera image */
    bsp_display_lock(0);
    camera_canvas = lv_canvas_create(lv_scr_act());
    lv_canvas_set_buffer(camera_canvas, cam_buff[0], BSP_LCD_H_RES, BSP_LCD_V_RES, LV_COLOR_FORMAT_RGB565);
    assert(camera_canvas);
    lv_obj_center(camera_canvas);
    bsp_display_unlock();

    /* Open video device */
    int fd = app_video_open(BSP_CAMERA_DEVICE, APP_VIDEO_FMT_RGB565);
    if (fd < 0) {
        ESP_LOGE(TAG, "Failed to open video device");
        ESP_LOGW(TAG, "Please, try to select another camera sensor in menuconfig.");
        return;
    }

    /* Initialize video capture device */
    ret = app_video_set_bufs(fd, NUM_BUFS, NULL);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to set video buffers: 0x%x", ret);
        return;
    }

    /* Register frame process callback */
    ret = app_video_register_frame_operation_cb(camera_video_frame_operation);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to register frame operation callback: 0x%x", ret);
        return;
    }

    /* Start video stream task */
    ret = app_video_stream_task_start(fd, 0);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to start video stream task: 0x%x", ret);
        return;
    }

    ESP_LOGI(TAG, "Camera example running.");
}
