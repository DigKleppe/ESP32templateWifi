#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <led_strip.h>
#include <stdio.h>
#include "driver/gpio.h"
#include "nvs_flash.h"
#include "esp_log.h"

#include "settings.h"
#include "wifiConnect.h"

#include <stdbool.h>

esp_err_t init_spiffs(void);

#define LED_TYPE LED_STRIP_WS2812
#define LED_GPIO GPIO_NUM_38 // GPIO_NUM_8
#define CONFIG_LED_STRIP_LEN 1

static const char *TAG = "main";

static const rgb_t colors[] = {
	{.r = 0x0f, .g = 0x0f, .b = 0x0f}, // wit
	{.r = 0x00, .g = 0x00, .b = 0x2f}, // blauw
	{.r = 0x00, .g = 0x2f, .b = 0x00}, // groen
	{.r = 0x2f, .g = 0x00, .b = 0x00}, // rood
	{.r = 0x2f, .g = 0x2f, .b = 0x00}, // geel
	{.r = 0x2f, .g = 0x00, .b = 0x2f}, // paars
	{.r = 0x00, .g = 0x2f, .b = 0x2f}, // cyaan
	{.r = 0x00, .g = 0x00, .b = 0x00}, // uit

};

#define COLORS_TOTAL (sizeof(colors) / sizeof(rgb_t))

void test(void *pvParameters) {
	led_strip_t strip = {
		.type = LED_TYPE,
		.is_rgbw = false,
#ifdef LED_STRIP_BRIGHTNESS
		.brightness = 255,
#endif
		.length = CONFIG_LED_STRIP_LEN,
		.gpio = LED_GPIO,
		.channel = RMT_CHANNEL_0,
		.buf = NULL,
	};

	ESP_ERROR_CHECK(led_strip_init(&strip));

	int c = 0;
	while (1) {
		switch ((int)connectStatus) {

		case CONNECTING:
			ESP_LOGI(TAG, "CONNECTING");
			c = 1; // blauw
			break;
            
            case WPS_ACTIVE:
            ESP_LOGI(TAG, "WPS_ACTIVE");
            c = 3; // rood
			break;

		case SMARTCONFIG_ACTIVE:
			ESP_LOGI(TAG, "SMARTCONFIG");
			c = 3; // rood
			break;
		case IP_RECEIVED:
			//     ESP_LOGI(TAG, "IP_RECEIVED");
			c = 2; // groen
			break;

		case CONNECTED:
			ESP_LOGI(TAG, "CONNECTED");
			c = 4; // geel
			break;

		default:
			ESP_LOGI(TAG, "default");
			break;
		}

		ESP_ERROR_CHECK(led_strip_fill(&strip, 0, strip.length, colors[c]));
		ESP_ERROR_CHECK(led_strip_flush(&strip));

		vTaskDelay(pdMS_TO_TICKS(1000));

		// if (++c >= COLORS_TOTAL)
		//     c = 0;
	}
}

extern "C" void app_main() {

	esp_err_t err = nvs_flash_init();
	if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
		ESP_ERROR_CHECK(nvs_flash_erase());
		err = nvs_flash_init();
		ESP_LOGI(TAG, "nvs flash erased");
	}
	ESP_ERROR_CHECK(err);

	ESP_ERROR_CHECK(esp_event_loop_create_default());
	ESP_ERROR_CHECK(init_spiffs());

	err = loadSettings();
  // strcpy ( wifiSettings.SSID, "kahjskljahs");  // test 

	wifiConnect();

	led_strip_install();
	xTaskCreate(test, "test", configMINIMAL_STACK_SIZE * 5, NULL, 5, NULL);
}
