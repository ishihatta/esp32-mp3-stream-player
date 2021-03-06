#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "driver/gpio.h"
#include "driver/i2s.h"
#include "app_wifi.h"
#include "DownloadTask.h"
#include "PlayerTask.h"
#include "main.h"

static const char *TAG = "main";

StreamBufferHandle_t sound_buffer;

void app_main(void) {
    ESP_LOGI(TAG, "Program start!");
    esp_err_t ret = nvs_flash_init();
    ESP_LOGI(TAG, "Initialize NVS: ret=%d", ret);
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    
    ESP_LOGI(TAG, "Initialize WiFi");
    app_wifi_initialise();

    ESP_LOGI(TAG, "Wait for connect to WiFi");
    app_wifi_wait_connected();

    ESP_LOGD(TAG, "Initialize stream buffer");
    sound_buffer = xStreamBufferCreate(64 * 1024, 1);

    ESP_LOGI(TAG, "Start download task");
    xTaskCreate(&DownloadTaskHandler, "download_task", 8 * 1024, NULL, 5, NULL);

    ESP_LOGI(TAG, "Start player task");
    xTaskCreate(&PlayerTaskHandler, "player_task", 64 * 1024, NULL, 5, NULL);
    
    while(1) vTaskDelay(1000 / portTICK_PERIOD_MS);
}
