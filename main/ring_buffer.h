#ifndef _RING_BUFFER_H_
#define _RING_BUFFER_H_

#include <stdio.h>
#include <stdint.h>
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

struct ring_buffer {
    xSemaphoreHandle mutex;
    uint8_t *buffer;
    size_t size;
    int write_position;
    int read_position;
    int writable_remain;
    int readable_remain;
};
typedef struct ring_buffer ring_buffer_t;

void ring_buffer_init(ring_buffer_t *ring_buffer, size_t size);
size_t ring_buffer_write(ring_buffer_t *ring_buffer, uint8_t *data, size_t size);
size_t ring_buffer_read(ring_buffer_t *ring_buffer, uint8_t *buffer, size_t size);

#endif
