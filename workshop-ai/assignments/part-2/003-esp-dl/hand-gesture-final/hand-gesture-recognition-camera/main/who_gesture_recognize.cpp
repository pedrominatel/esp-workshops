#include "who_gesture_recognize.hpp"
#include "esp_heap_caps.h"
#include "esp_log.h"
#include "frame_cap_pipeline.hpp"
#include <cstring>

static const char *TAG = "hand_gesture_recognition";

namespace who {
namespace gesture {

void WhoGestureRecognize::preload_models(HandDetect **hand_detect, HandGestureRecognizer **recognizer)
{
    ESP_LOGI(TAG, "Loading hand detect model...");
    *hand_detect = new HandDetect(HandDetect::ESPDET_PICO_224_224_HAND, false);
    *recognizer = new HandGestureRecognizer(HandGestureCls::MOBILENETV2_0_5_S8_V1);
    ESP_LOGI(TAG, "Hand detect model loaded");
}

WhoGestureRecognize::WhoGestureRecognize(const std::string &name,
                                         frame_cap::WhoFrameCapNode *frame_cap_node,
                                         HandDetect *hand_detect,
                                         HandGestureRecognizer *recognizer) :
    task::WhoTask(name),
    m_frame_cap_node(frame_cap_node),
    m_hand_detect(hand_detect),
    m_recognizer(recognizer),
    m_copy_buf(nullptr),
    m_interval(0)
{
    // Pre-allocate a private frame buffer so inference never touches resize node memory.
    // The resize node rotates between ringbuf+1 PSRAM buffers; copying before inference
    // eliminates the data-race that caused crashes with the previous JPEG approach.
    const size_t buf_size = static_cast<size_t>(PROCESS_FRAME_W) * PROCESS_FRAME_H * 2;
    m_copy_buf = heap_caps_malloc(buf_size, MALLOC_CAP_SPIRAM);
    ESP_ERROR_CHECK(m_copy_buf ? ESP_OK : ESP_ERR_NO_MEM);
    frame_cap_node->add_new_frame_signal_subscriber(this);
}

WhoGestureRecognize::~WhoGestureRecognize()
{
    heap_caps_free(m_copy_buf);
    delete m_recognizer;
    delete m_hand_detect;
}

void WhoGestureRecognize::set_fps(float fps)
{
    if (fps > 0) {
        m_interval = pdMS_TO_TICKS(static_cast<int>(1000.f / fps));
    }
}

void WhoGestureRecognize::task()
{
    const size_t expected_len = static_cast<size_t>(PROCESS_FRAME_W) * PROCESS_FRAME_H * 2;

    TickType_t last_wake_time = xTaskGetTickCount();
    while (true) {
        EventBits_t event_bits =
            xEventGroupWaitBits(m_event_group, NEW_FRAME | TASK_PAUSE | TASK_STOP, pdTRUE, pdFALSE, portMAX_DELAY);
        if (event_bits & TASK_STOP) {
            break;
        } else if (event_bits & TASK_PAUSE) {
            xEventGroupSetBits(m_event_group, TASK_PAUSED);
            EventBits_t pause_event_bits =
                xEventGroupWaitBits(m_event_group, TASK_RESUME | TASK_STOP, pdTRUE, pdFALSE, portMAX_DELAY);
            if (pause_event_bits & TASK_STOP) {
                break;
            } else {
                last_wake_time = xTaskGetTickCount();
                continue;
            }
        }

        auto fb = m_frame_cap_node->cam_fb_peek();
        if (!fb || !fb->buf || fb->len != expected_len) {
            continue;
        }

        // memcpy (~1 ms for 460 KB at PSRAM speed) before the resize node may
        // recycle this buffer for the next camera frame.
        memcpy(m_copy_buf, fb->buf, expected_len);

        dl::image::img_t img = {
            .data = m_copy_buf,
            .width = PROCESS_FRAME_W,
            .height = PROCESS_FRAME_H,
            .pix_type = dl::image::DL_IMAGE_PIX_TYPE_RGB565,
        };

        auto detect_res = m_hand_detect->run(img);
        auto results = m_recognizer->recognize(img, detect_res);

        for (const auto &res : results) {
            ESP_LOGI(TAG, "category: %s, score: %.3f", res.cat_name, res.score);
        }

        if (m_interval) {
            vTaskDelayUntil(&last_wake_time, m_interval);
        }
    }
    xEventGroupSetBits(m_event_group, TASK_STOPPED);
    vTaskDelete(NULL);
}

bool WhoGestureRecognize::run(const configSTACK_DEPTH_TYPE uxStackDepth,
                               UBaseType_t uxPriority,
                               const BaseType_t xCoreID)
{
    return task::WhoTask::run(uxStackDepth, uxPriority, xCoreID);
}

bool WhoGestureRecognize::stop_async()
{
    if (task::WhoTask::stop_async()) {
        xTaskAbortDelay(m_task_handle);
        return true;
    }
    return false;
}

bool WhoGestureRecognize::pause_async()
{
    if (task::WhoTask::pause_async()) {
        xTaskAbortDelay(m_task_handle);
        return true;
    }
    return false;
}

} // namespace gesture
} // namespace who
