#include <string.h>
#include <stdlib.h>
#include "esp_log.h"
#include "ring_buffer.h"

static const char *TAG = "ring_buffer";

void ring_buffer_init(ring_buffer_t *ring_buffer, size_t size) {
    ring_buffer->mutex = xSemaphoreCreateMutex();
    ring_buffer->buffer = malloc(size);
    if (ring_buffer->buffer == NULL) {
        ESP_LOGE(TAG, "CANNOT ALLOCATE BUFFER MEMORY!");
    }
    ring_buffer->size = size;
    ring_buffer->read_position = 0;
    ring_buffer->write_position = 0;
    ring_buffer->readable_remain = 0;
    ring_buffer->writable_remain = size;
}

size_t ring_buffer_write(ring_buffer_t *ring_buffer, uint8_t *data, size_t size) {
    xSemaphoreTake(ring_buffer->mutex, portMAX_DELAY);

    if (ring_buffer->writable_remain == 0) {
        xSemaphoreGive(ring_buffer->mutex);
        return 0;
    }

    // Real write size
    size_t write_size = size;
    if (write_size > ring_buffer->writable_remain) write_size = ring_buffer->writable_remain;

    if (ring_buffer->write_position + write_size > ring_buffer->size) {
        size_t size1 = ring_buffer->size - ring_buffer->write_position;
        size_t size2 = write_size - size1;
        memcpy(ring_buffer->buffer + ring_buffer->write_position, data, size1);
        memcpy(ring_buffer->buffer, data + size1, size2);
        ring_buffer->write_position = size2;
    } else {
        memcpy(ring_buffer->buffer + ring_buffer->write_position, data, write_size);
        ring_buffer->write_position += write_size;
    }

    ring_buffer->writable_remain -= write_size;
    ring_buffer->readable_remain += write_size;

    xSemaphoreGive(ring_buffer->mutex);

    return write_size;
}

size_t ring_buffer_read(ring_buffer_t *ring_buffer, uint8_t *buffer, size_t size) {
    xSemaphoreTake(ring_buffer->mutex, portMAX_DELAY);
    
    if (ring_buffer->readable_remain == 0) {
        xSemaphoreGive(ring_buffer->mutex);
        return 0;
    }

    // Real read size
    size_t read_size = size;
    if (read_size > ring_buffer->readable_remain) read_size = ring_buffer->readable_remain;

    if (ring_buffer->read_position + read_size > ring_buffer->size) {
        size_t size1 = ring_buffer->size - ring_buffer->read_position;
        size_t size2 = read_size - size1;
        memcpy(buffer, ring_buffer->buffer + ring_buffer->read_position, size1);
        memcpy(buffer + size1, ring_buffer->buffer, size2);
        ring_buffer->read_position = size2;
    } else {
        memcpy(buffer, ring_buffer->buffer + ring_buffer->read_position, read_size);
        ring_buffer->read_position += read_size;
    }

    ring_buffer->readable_remain -= read_size;
    ring_buffer->writable_remain += read_size;

    xSemaphoreGive(ring_buffer->mutex);

    return read_size;
}