/*
 * SPDX-FileCopyrightText: 2025 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef APP_VIDEO_H
#define APP_VIDEO_H

#include "esp_err.h"
#include "linux/videodev2.h"
#include "esp_video_device.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    APP_VIDEO_FMT_RAW8  = V4L2_PIX_FMT_SBGGR8,
    APP_VIDEO_FMT_RAW10 = V4L2_PIX_FMT_SBGGR10,
    APP_VIDEO_FMT_GREY  = V4L2_PIX_FMT_GREY,
    APP_VIDEO_FMT_RGB565 = V4L2_PIX_FMT_RGB565,
    APP_VIDEO_FMT_RGB888 = V4L2_PIX_FMT_RGB24,
    APP_VIDEO_FMT_YUV422 = V4L2_PIX_FMT_YUV422P,
    APP_VIDEO_FMT_YUV420 = V4L2_PIX_FMT_YUV420,
} video_fmt_t;

typedef void (*app_video_frame_operation_cb_t)(uint8_t *camera_buf, uint8_t camera_buf_index,
        uint32_t camera_buf_hes, uint32_t camera_buf_ves, size_t camera_buf_len);

/**
 * @brief Open a video device and configure format and flip controls.
 *
 * @param dev      Device path string (e.g. BSP_CAMERA_DEVICE).
 * @param init_fmt Desired pixel format (video_fmt_t).
 * @return File descriptor on success, -1 on failure.
 */
int app_video_open(char *dev, video_fmt_t init_fmt);

/**
 * @brief Allocate or register frame buffers with the video device.
 *
 * @param video_fd File descriptor for the video device.
 * @param fb_num   Number of frame buffers.
 * @param fb       Array of user pointers (NULL for MMAP mode).
 * @return ESP_OK on success, ESP_FAIL on failure.
 */
esp_err_t app_video_set_bufs(int video_fd, uint32_t fb_num, const void **fb);

/**
 * @brief Retrieve mmap'd frame buffer pointers after app_video_set_bufs.
 *
 * @param fb_num Number of frame buffers.
 * @param fb     Output array to receive buffer pointers.
 * @return ESP_OK on success, ESP_FAIL on failure.
 */
esp_err_t app_video_get_bufs(int fb_num, void **fb);

/**
 * @brief Start the video stream task pinned to a specific core.
 *
 * @param video_fd File descriptor for the video device.
 * @param core_id  Core ID to which the task will be pinned.
 * @return ESP_OK on success, ESP_FAIL on failure.
 */
esp_err_t app_video_stream_task_start(int video_fd, int core_id);

/**
 * @brief Stop the video stream task.
 *
 * @param video_fd File descriptor for the video device.
 * @return ESP_OK on success.
 */
esp_err_t app_video_stream_task_stop(int video_fd);

/**
 * @brief Register a callback for frame operations.
 *
 * @param operation_cb Callback function to register.
 * @return ESP_OK on success.
 */
esp_err_t app_video_register_frame_operation_cb(app_video_frame_operation_cb_t operation_cb);

/**
 * @brief Wait for the video stream task to stop.
 *
 * @return ESP_OK on success.
 */
esp_err_t app_video_wait_video_stop(void);

/**
 * @brief Close the video device.
 *
 * @param video_fd File descriptor to close.
 * @return ESP_OK on success.
 */
esp_err_t app_video_close(int video_fd);

#ifdef __cplusplus
}
#endif
#endif