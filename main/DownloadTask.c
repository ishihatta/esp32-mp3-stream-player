#include <string.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_system.h"
#include "esp_http_client.h"
#include "nvs_flash.h"
#include "app_wifi.h"
#include "main.h"

#define DOWNLOAD_URL "http://192.168.10.118:8887/Dai9.mp3"
#define MAX_HTTP_RECV_BUFFER 512

static const char *TAG = "DownloadTask";

static void http_perform_as_stream_reader()
{
    uint8_t *buffer = malloc(MAX_HTTP_RECV_BUFFER + 1);
    if (buffer == NULL) {
        ESP_LOGE(TAG, "Cannot malloc http receive buffer");
        return;
    }
    esp_http_client_config_t config = {
        .url = DOWNLOAD_URL,
    };
    esp_http_client_handle_t client = esp_http_client_init(&config);
    esp_err_t err;
    if ((err = esp_http_client_open(client, 0)) != ESP_OK) {
        ESP_LOGE(TAG, "Failed to open HTTP connection: %s", esp_err_to_name(err));
        free(buffer);
        return;
    }
    int content_length =  esp_http_client_fetch_headers(client);
    int total_read_len = 0, read_len;
    while (total_read_len < content_length) {
        read_len = esp_http_client_read(client, (char*)buffer, MAX_HTTP_RECV_BUFFER);
        if (read_len <= 0) {
            ESP_LOGE(TAG, "Error read data");
            break;
        }
        total_read_len += read_len;

        xStreamBufferSend(sound_buffer, buffer, read_len, portMAX_DELAY);
    }
    esp_http_client_close(client);
    esp_http_client_cleanup(client);
    free(buffer);
}

void DownloadTaskHandler(void *temp) {
    http_perform_as_stream_reader();

    while(1) vTaskDelay(1000 / portTICK_PERIOD_MS);
}
