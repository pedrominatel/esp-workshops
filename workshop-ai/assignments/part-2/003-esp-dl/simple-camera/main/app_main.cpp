#include "frame_cap_pipeline.hpp"
#include "who_frame_lcd_disp.hpp"
#include "who_yield2idle.hpp"
#include "bsp/esp-bsp.h"

using namespace who;
using namespace who::frame_cap;
using namespace who::lcd_disp;

extern "C" void app_main(void)
{
    vTaskPrioritySet(xTaskGetCurrentTaskHandle(), 5);

#ifdef BSP_BOARD_ESP32_S3_EYE
    ESP_ERROR_CHECK(bsp_leds_init());
    ESP_ERROR_CHECK(bsp_led_set(BSP_LED_GREEN, false));
#endif

#if CONFIG_IDF_TARGET_ESP32S3
    auto *frame_cap = get_dvp_frame_cap_pipeline();
#elif CONFIG_IDF_TARGET_ESP32P4
    auto *frame_cap = get_mipi_csi_frame_cap_pipeline();
    // auto *frame_cap = get_uvc_frame_cap_pipeline();
#endif

    auto *lcd_disp = new WhoFrameLCDDisp("LCDDisp", frame_cap->get_last_node());

    bool ret = WhoYield2Idle::get_instance()->run();
    for (const auto &node : frame_cap->get_all_nodes()) {
        ret &= node->run(4096, 2, 0);
    }
    ret &= lcd_disp->run(2560, 2, 0);
    ESP_ERROR_CHECK(ret ? ESP_OK : ESP_FAIL);
}
