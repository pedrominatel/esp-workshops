#include "frame_cap_pipeline.hpp"
#include "who_gesture_recognize.hpp"
#include "who_yield2idle.hpp"
#include "bsp/esp-bsp.h"
#include "esp_heap_caps.h"
#include "esp_log.h"

using namespace who;
using namespace who::frame_cap;
using namespace who::gesture;

extern "C" void app_main(void)
{
    vTaskPrioritySet(xTaskGetCurrentTaskHandle(), 5);

#ifdef BSP_BOARD_ESP32_S3_EYE
    ESP_ERROR_CHECK(bsp_leds_init());
    ESP_ERROR_CHECK(bsp_led_set(BSP_LED_GREEN, false));
#endif

#if CONFIG_IDF_TARGET_ESP32S3
    auto *frame_cap = get_dvp_frame_cap_pipeline();
    auto *process_node = get_processing_frame_node(frame_cap);
#elif CONFIG_IDF_TARGET_ESP32P4
    auto *frame_cap = get_mipi_csi_frame_cap_pipeline();
    auto *process_node = frame_cap->get_last_node();
#endif

    ESP_LOGI("app_main", "Free heap before models: %u B  PSRAM: %u B",
             esp_get_free_heap_size(),
             heap_caps_get_free_size(MALLOC_CAP_SPIRAM));

    HandDetect *hand_detect = nullptr;
    HandGestureRecognizer *recognizer = nullptr;
    WhoGestureRecognize::preload_models(&hand_detect, &recognizer);

    auto *gesture_task = new WhoGestureRecognize("GestureRecognize", process_node, hand_detect, recognizer);
    gesture_task->set_fps(1);

    ESP_LOGI("app_main", "Free heap before tasks: %u B  PSRAM: %u B",
             esp_get_free_heap_size(),
             heap_caps_get_free_size(MALLOC_CAP_SPIRAM));

    // Run all tasks on Core 0: HandDetect uses Xtensa TIE/SIMD instructions
    // that corrupt Core 0's interrupt WDT state when running on Core 1.
    // Keeping inference co-located with Core 0's WDT tick hook avoids this.
    bool ret = WhoYield2Idle::get_instance()->run();
    for (const auto &node : frame_cap->get_all_nodes()) {
        ret &= node->run(32768, 2, 0);
    }
    ret &= gesture_task->run(32768, 1, 0);
    ESP_ERROR_CHECK(ret ? ESP_OK : ESP_FAIL);
}
