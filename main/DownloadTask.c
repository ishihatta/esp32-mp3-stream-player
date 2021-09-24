#include <string.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "app_wifi.h"
#include "main.h"

#include "esp_http_client.h"

#define DOWNLOAD_URL "http://192.168.10.118:8887/Dai9.mp3"
#define MAX_HTTP_RECV_BUFFER 512

static const char *TAG = "DownloadTask";

esp_err_t _http_event_handler(esp_http_client_event_t *evt)
{
    switch(evt->event_id) {
        case HTTP_EVENT_ERROR:
            ESP_LOGD(TAG, "HTTP_EVENT_ERROR");
            break;
        case HTTP_EVENT_ON_CONNECTED:
            ESP_LOGD(TAG, "HTTP_EVENT_ON_CONNECTED");
            break;
        case HTTP_EVENT_HEADER_SENT:
            ESP_LOGD(TAG, "HTTP_EVENT_HEADER_SENT");
            break;
        case HTTP_EVENT_ON_HEADER:
            ESP_LOGD(TAG, "HTTP_EVENT_ON_HEADER, key=%s, value=%s", evt->header_key, evt->header_value);
            break;
        case HTTP_EVENT_ON_DATA:
            ESP_LOGD(TAG, "HTTP_EVENT_ON_DATA, len=%d", evt->data_len);
            if (!esp_http_client_is_chunked_response(evt->client)) {
                // Write out data
                // printf("%.*s", evt->data_len, (char*)evt->data);
            }

            break;
        case HTTP_EVENT_ON_FINISH:
            ESP_LOGD(TAG, "HTTP_EVENT_ON_FINISH");
            break;
        case HTTP_EVENT_DISCONNECTED:
            ESP_LOGD(TAG, "HTTP_EVENT_DISCONNECTED");
            break;
    }
    return ESP_OK;
}

static void http_perform_as_stream_reader()
{
    uint8_t *buffer = malloc(MAX_HTTP_RECV_BUFFER + 1);
    if (buffer == NULL) {
        ESP_LOGE(TAG, "Cannot malloc http receive buffer");
        return;
    }
    esp_http_client_config_t config = {
        .url = DOWNLOAD_URL,
        .event_handler = _http_event_handler,
    };
    esp_http_client_handle_t client = esp_http_client_init(&config);
    esp_err_t err;
    if ((err = esp_http_client_open(client, 0)) != ESP_OK) {
        ESP_LOGE(TAG, "Failed to open HTTP connection: %s", esp_err_to_name(err));
        free(buffer);
        return;
    }
    int content_length =  esp_http_client_fetch_headers(client);
    int total_read_len = 0, read_len, write_len, write_remain, write_position;
    while (total_read_len < content_length) {
        read_len = esp_http_client_read(client, (char*)buffer, MAX_HTTP_RECV_BUFFER);
        if (read_len <= 0) {
            ESP_LOGE(TAG, "Error read data");
            break;
        }
        total_read_len += read_len;

        write_remain = read_len;
        write_position = 0;
        while (true) {
            write_len = ring_buffer_write(&sound_buffer, buffer + write_position, write_remain);
            write_position += write_len;
            write_remain -= write_len;
            if (write_remain == 0) break;
            vTaskDelay(10 / portTICK_PERIOD_MS);
        }
    }
    esp_http_client_close(client);
    esp_http_client_cleanup(client);
    free(buffer);
}

void DownloadTaskHandler(void *temp) {
    http_perform_as_stream_reader();

    while(1) vTaskDelay(1000 / portTICK_PERIOD_MS);
}
