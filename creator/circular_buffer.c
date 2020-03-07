#include <stdio.h>
#include <stdlib.h>
#include <math.h> // math for exponential function
#include <time.h> // time for seeding
#include <pthread.h> // include pthread functions and structures
#include <semaphore.h> // include semaphores
#include <unistd.h> // include sleep
#include <stdint.h>
#include <stddef.h>
#include <assert.h>
#include "circular_buffer.h"


// The definition of our circular buffer structure is hidden from the user
struct circular_buf_t {
    uint8_t * buffer;
    size_t head;
    size_t tail;
    size_t max; //of the buffer
    bool full;
};

#pragma mark - Private Functions -

static void advance_pointer(cbuf_handle_t cbuf)
{
    assert(cbuf);

    if(cbuf->full)
    {
        cbuf->tail = (cbuf->tail + 1) % cbuf->max;
    }

    cbuf->head = (cbuf->head + 1) % cbuf->max;

    // We mark full because we will advance tail on the next time around
    cbuf->full = (cbuf->head == cbuf->tail);
}

static void retreat_pointer(cbuf_handle_t cbuf)
{
    assert(cbuf);

    cbuf->full = false;
    cbuf->tail = (cbuf->tail + 1) % cbuf->max;
}

#pragma mark - APIs -

// Pass in a storage buffer and size, returns a circular buffer handler
cbuf_handle_t circular_buf_init(uint8_t* buffer, size_t size)
{
    assert(buffer && size);

    cbuf_handle_t cbuf = malloc(sizeof(circular_buf_t));
    assert(cbuf);

    cbuf->buffer = buffer;
    cbuf->max = size;
    circular_buf_reset(cbuf);

    assert(circular_buf_empty(cbuf));

    return cbuf;
}

//free buffer but not physical memory involved
void circular_buf_free(cbuf_handle_t cbuf)
{
    assert(cbuf);
    free(cbuf);
}

// Reset the circular buffer to empty, head == tail.
void circular_buf_reset(cbuf_handle_t cbuf)
{
    assert(cbuf);

    cbuf->head = 0;
    cbuf->tail = 0;
    cbuf->full = false;
}

// Returns the current number of elements in the buffer
size_t circular_buf_size(cbuf_handle_t cbuf)
{
    assert(cbuf);

    size_t size = cbuf->max;

    if(!cbuf->full)
    {
        if(cbuf->head >= cbuf->tail)
        {
            size = (cbuf->head - cbuf->tail);
        }
        else
        {
            size = (cbuf->max + cbuf->head - cbuf->tail);
        }

    }

    return size;
}

// Returns the maximum capacity of the buffer
size_t circular_buf_capacity(cbuf_handle_t cbuf)
{
    assert(cbuf);

    return cbuf->max;
}

// continues to add data if the buffer is full
void circular_buf_put(cbuf_handle_t cbuf, uint8_t data)
{
    assert(cbuf && cbuf->buffer);

    cbuf->buffer[cbuf->head] = data;

    advance_pointer(cbuf);
}

// rejects new data if the buffer is full
int circular_buf_put2(cbuf_handle_t cbuf, uint8_t data)
{
    int r = -1;

    assert(cbuf && cbuf->buffer);

    if(!circular_buf_full(cbuf))
    {
        cbuf->buffer[cbuf->head] = data;
        advance_pointer(cbuf);
        r = 0;
    }

    return r;
}

// retrieve a value from the buffer
int circular_buf_get(cbuf_handle_t cbuf, uint8_t * data)
{
    assert(cbuf && data && cbuf->buffer);

    int r = -1;

    if(!circular_buf_empty(cbuf))
    {
        *data = cbuf->buffer[cbuf->tail];
        retreat_pointer(cbuf);

        r = 0;
    }

    return r;
}

// checks if the buffer is empty
bool circular_buf_empty(cbuf_handle_t cbuf)
{
    assert(cbuf);

    return (!cbuf->full && (cbuf->head == cbuf->tail));
}

// checks if the buffer is full
bool circular_buf_full(cbuf_handle_t cbuf)
{
    assert(cbuf);

    return cbuf->full;
}
