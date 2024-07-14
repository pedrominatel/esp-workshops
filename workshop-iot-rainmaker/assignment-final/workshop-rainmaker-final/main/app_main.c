/* RainMaker Workshop Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include <string.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_log.h>
#include <nvs_flash.h>

#include <esp_rmaker_core.h>
#include <esp_rmaker_standard_params.h>
#include <esp_rmaker_standard_devices.h>
#include <esp_rmaker_standard_types.h>
#include <esp_rmaker_schedule.h>
#include <esp_rmaker_console.h>
#include <esp_rmaker_scenes.h>

#include <app_wifi.h>
#include <app_insights.h>

#include "app_priv.h"

static const char *TAG = "workshop";

esp_rmaker_device_t *light_device;

static esp_err_t bulk_write_cb(const esp_rmaker_device_t *device, const esp_rmaker_param_write_req_t write_req[],
        uint8_t count, void *priv_data, esp_rmaker_write_ctx_t *ctx)
{
    if (ctx) {
        ESP_LOGI(TAG, "Received write request via : %s", esp_rmaker_device_cb_src_to_str(ctx->src));
    }
    
    ESP_LOGI(TAG, "Light received %d params in write", count);
    for (int i = 0; i < count; i++) {
        const esp_rmaker_param_t *param = write_req[i].param;
        esp_rmaker_param_val_t val = write_req[i].val;
        const char *device_name = esp_rmaker_device_get_name(device);
        const char *param_name = esp_rmaker_param_get_name(param);
        if (strcmp(param_name, ESP_RMAKER_DEF_POWER_NAME) == 0) {
            ESP_LOGI(TAG, "Received value = %s for %s - %s",
                    val.val.b? "true" : "false", device_name, param_name);
            app_light_set_power(val.val.b);
        } else if (strcmp(param_name, ESP_RMAKER_DEF_BRIGHTNESS_NAME) == 0) {
            ESP_LOGI(TAG, "Received value = %d for %s - %s",
                    val.val.i, device_name, param_name);
            app_light_set_brightness(val.val.i);
        } else if (strcmp(param_name, ESP_RMAKER_DEF_HUE_NAME) == 0) {
            ESP_LOGI(TAG, "Received value = %d for %s - %s",
                    val.val.i, device_name, param_name);
            app_light_set_hue(val.val.i);
        } else if (strcmp(param_name, ESP_RMAKER_DEF_SATURATION_NAME) == 0) {
            ESP_LOGI(TAG, "Received value = %d for %s - %s",
                    val.val.i, device_name, param_name);
            app_light_set_saturation(val.val.i);
        } else {
            ESP_LOGI(TAG, "Updating for %s", param_name);
        }
        esp_rmaker_param_update(param, val);
    }
    return ESP_OK;
}

void device_create_lightbulb(esp_rmaker_node_t *node)
{

    light_device = esp_rmaker_lightbulb_device_create("Light", NULL, DEFAULT_POWER);
    esp_rmaker_device_add_bulk_cb(light_device, bulk_write_cb, NULL);

    esp_rmaker_device_add_param(light_device, esp_rmaker_brightness_param_create(ESP_RMAKER_DEF_BRIGHTNESS_NAME, DEFAULT_BRIGHTNESS));
    esp_rmaker_device_add_param(light_device, esp_rmaker_hue_param_create(ESP_RMAKER_DEF_HUE_NAME, DEFAULT_HUE));
    esp_rmaker_device_add_param(light_device, esp_rmaker_saturation_param_create(ESP_RMAKER_DEF_SATURATION_NAME, DEFAULT_SATURATION));

    esp_rmaker_node_add_device(node, light_device);
}

static esp_err_t write_cb(const esp_rmaker_device_t *device, const esp_rmaker_param_t *param,
            const esp_rmaker_param_val_t val, void *priv_data, esp_rmaker_write_ctx_t *ctx)
{
    if (ctx) {
        ESP_LOGI(TAG, "Received write request via : %s", esp_rmaker_device_cb_src_to_str(ctx->src));
    }
    return ESP_OK;
}

void device_create_switch(esp_rmaker_node_t *node)
{

    esp_rmaker_device_t *switch_device = esp_rmaker_device_create("Switch", ESP_RMAKER_DEVICE_SWITCH, NULL);
    esp_rmaker_device_add_cb(switch_device, write_cb, NULL);

    esp_rmaker_param_t *power_param = esp_rmaker_param_create("Power", ESP_RMAKER_PARAM_POWER, esp_rmaker_bool(false),
    			PROP_FLAG_READ | PROP_FLAG_WRITE);
    
    esp_rmaker_param_add_ui_type(power_param, ESP_RMAKER_UI_TOGGLE);
    esp_rmaker_device_add_param(switch_device, power_param);

    esp_rmaker_node_add_device(node, switch_device);
}

void device_create_thermostat(esp_rmaker_node_t *node)
{

    esp_rmaker_device_t *thermo_device = esp_rmaker_device_create("Thermostat", ESP_RMAKER_DEVICE_THERMOSTAT, NULL);
    esp_rmaker_device_add_cb(thermo_device, write_cb, NULL);

    esp_rmaker_param_t *power_param = esp_rmaker_param_create("Power", ESP_RMAKER_PARAM_POWER, esp_rmaker_bool(false),
				PROP_FLAG_READ | PROP_FLAG_WRITE);
    
    esp_rmaker_param_add_ui_type(power_param, ESP_RMAKER_UI_TOGGLE);
    esp_rmaker_device_add_param(thermo_device, power_param);

	esp_rmaker_param_t *temp_param = esp_rmaker_param_create(ESP_RMAKER_DEF_TEMPERATURE_NAME, ESP_RMAKER_PARAM_TEMPERATURE, esp_rmaker_int(20),
	            PROP_FLAG_READ);
	esp_rmaker_param_add_ui_type(temp_param, ESP_RMAKER_UI_TEXT);
	esp_rmaker_device_add_param(thermo_device, temp_param);

	esp_rmaker_param_t *setpoint_param = esp_rmaker_param_create("Temperature Set", ESP_RMAKER_PARAM_TEMPERATURE, esp_rmaker_int(20),
	            PROP_FLAG_READ | PROP_FLAG_WRITE);
	esp_rmaker_param_add_ui_type(setpoint_param, ESP_RMAKER_UI_SLIDER);
	esp_rmaker_param_add_bounds(setpoint_param, esp_rmaker_int(15), esp_rmaker_int(30), esp_rmaker_int(1));
	esp_rmaker_device_add_param(thermo_device, setpoint_param);

    esp_rmaker_node_add_device(node, thermo_device);
}

void app_main()
{

    esp_rmaker_console_init();
    app_driver_init();

    /* Initialize NVS. */
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK( err );

    app_wifi_init();

    esp_rmaker_config_t rainmaker_cfg = {
        .enable_time_sync = false,
    };
    
    esp_rmaker_node_t *node = esp_rmaker_node_init(&rainmaker_cfg, "Espressif Workshop Light", "Lightbulb");
    if (!node) {
        ESP_LOGE(TAG, "Could not initialise node. Aborting!!!");
        vTaskDelay(5000/portTICK_PERIOD_MS);
        abort();
    }

	device_create_lightbulb(node);

	device_create_switch(node);
	
	device_create_thermostat(node);

    esp_rmaker_ota_enable_default();

    esp_rmaker_timezone_service_enable();

    esp_rmaker_schedule_enable();

    esp_rmaker_scenes_enable();

    app_insights_enable();

    esp_rmaker_start();

    err = app_wifi_start(POP_TYPE_RANDOM);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Could not start Wifi. Aborting!!!");
        vTaskDelay(5000/portTICK_PERIOD_MS);
        abort();
    }
}
