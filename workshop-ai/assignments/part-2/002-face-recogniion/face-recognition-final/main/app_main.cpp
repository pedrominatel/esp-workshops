#include "frame_cap_pipeline.hpp"
#include "who_recognition_app_lcd.hpp"
#include "who_spiflash_fatfs.hpp"

using namespace who::frame_cap;
using namespace who::app;
using namespace who::recognition;

/*
 * Extends the stock LCD recognition app to drive the green LED based on
 * recognition results (known face) rather than detection alone (any face).
 *
 * Pipeline overview:
 *   Camera -> Detect task (finds faces every frame)
 *           -> Recognition task (matches face against enrolled database)
 *           -> LCD display (bounding boxes + name/similarity text)
 *
 * Detection and recognition are separate tasks in esp-who. Detection runs
 * continuously; recognition is triggered on demand via the RECOGNIZE event.
 */
class WhoRecognitionAppLCDWithCallback : public WhoRecognitionAppLCD {
public:
    WhoRecognitionAppLCDWithCallback(who::frame_cap::WhoFrameCap *frame_cap) : WhoRecognitionAppLCD(frame_cap)
    {
        /*
         * Re-register callbacks so our overrides are actually invoked.
         * The base class binds with std::bind(&WhoRecognitionAppLCD::...),
         * which always calls the base methods and bypasses virtual dispatch.
         */
        auto *recognition_task = m_recognition->get_recognition_task();
        auto *detect_task = m_recognition->get_detect_task();
        recognition_task->set_recognition_result_cb(
            [this](const std::string &result) { recognition_result_cb(result); });
        recognition_task->set_detect_result_cb(
            [this](const who::detect::WhoDetect::result_t &result) { detect_result_cb(result); });
        detect_task->set_detect_result_cb(
            [this](const who::detect::WhoDetect::result_t &result) { detect_result_cb(result); });
    }

protected:
    /*
     * Called after a recognition attempt completes.
     * esp-who result strings:
     *   "id: N, sim: X.XX" -> face matched an enrolled identity
     *   "who?"             -> face seen but not in the database
     *   other strings      -> enroll/delete feedback (LED left unchanged)
     */
    void recognition_result_cb(const std::string &result) override
    {
        if (result.find("sim:") != std::string::npos) {
            ESP_LOGI("RECOGNITION", "Face recognized: %s", result.c_str());
            bsp_led_set(BSP_LED_GREEN, true);
        } else if (result == "who?") {
            bsp_led_set(BSP_LED_GREEN, false);
        }

        // Keep the name/similarity label on the LCD in sync.
        WhoRecognitionAppLCD::recognition_result_cb(result);
    }

    /*
     * Called on every frame by the detect task with bounding-box results.
     * Used here to turn the LED off when no face is visible and to request
     * a recognition attempt when a face is present.
     */
    void detect_result_cb(const who::detect::WhoDetect::result_t &result) override
    {
        if (result.det_res.empty()) {
            bsp_led_set(BSP_LED_GREEN, false);
        } else {
            /*
             * Throttle recognition requests to avoid flooding the recognition
             * task. Each RECOGNIZE event temporarily replaces the detect
             * callback; triggering too often can stall detection.
             */
            TickType_t now = xTaskGetTickCount();
            if (now - m_last_recognition_trigger > pdMS_TO_TICKS(500)) {
                m_last_recognition_trigger = now;
                trigger_recognition();
            }
        }

        // Keep the red bounding boxes on the LCD in sync.
        WhoRecognitionAppLCD::detect_result_cb(result);
    }

private:
    TickType_t m_last_recognition_trigger = 0;

    // Signal the recognition task to compare the next detected face to the database.
    void trigger_recognition()
    {
        auto *recognition_task = m_recognition->get_recognition_task();
        if (recognition_task->is_active()) {
            xEventGroupSetBits(recognition_task->get_event_group(), WhoRecognitionCore::RECOGNIZE);
        }
    }
};

extern "C" void app_main(void)
{
    vTaskPrioritySet(xTaskGetCurrentTaskHandle(), 5);

    // face.db (enrolled identities) is stored on the SPI flash partition.
    ESP_ERROR_CHECK(fatfs_flash_mount());
    ESP_ERROR_CHECK(bsp_leds_init());
    ESP_ERROR_CHECK(bsp_led_set(BSP_LED_GREEN, false));

    auto frame_cap = get_dvp_frame_cap_pipeline();
    auto recognition_app = new WhoRecognitionAppLCDWithCallback(frame_cap);

    // Starts camera, LCD, detect, and recognition FreeRTOS tasks.
    recognition_app->run();
}
