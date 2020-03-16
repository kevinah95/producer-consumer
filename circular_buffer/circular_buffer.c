#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <assert.h>
#include <string.h>
#include "circular_buffer.h"


// The definition of our circular buffer structure is hidden from the user
struct circular_buf_t
{
    char buffer[4096][256];
    size_t head;
    size_t tail;
    size_t max; //of the buffer
    bool full;
};

#pragma mark - Private Functions -

void advance_pointer(struct circular_buf_t *cbuf)
{

    if (cbuf->full)
    {
        cbuf->tail = (cbuf->tail + 1) % cbuf->max;
    }

    cbuf->head = (cbuf->head + 1) % cbuf->max;

    // We mark full because we will advance tail on the next time around
    cbuf->full = (cbuf->head == cbuf->tail);
}

int circular_buf_put(struct circular_buf_t *cbuf, char *data)
{
    int r = -1;

    if (!circular_buf_full(cbuf))
    {
        strcpy(cbuf->buffer[cbuf->head], data);
        advance_pointer(cbuf);
        r = 0;
    }

    return r;
}

static void retreat_pointer(cbuf_handle_t cbuf)
{
    assert(cbuf);

    cbuf->full = false;
    cbuf->tail = (cbuf->tail + 1) % cbuf->max;
}

#pragma mark - APIs -


void circular_buf_free(cbuf_handle_t cbuf)
{
    assert(cbuf);
    free(cbuf);
}

void print_buffer_status(struct circular_buf_t *cbuf)
{
    printf("Full: %d, empty: %d, size: %zu\n",
           circular_buf_full(cbuf),
           circular_buf_empty(cbuf),
           circular_buf_size(cbuf));
}

void circular_buf_reset(struct circular_buf_t * cbuf)
{
    cbuf->head = 0;
    cbuf->tail = 0;
    cbuf->full = false;
}

size_t circular_buf_size(struct circular_buf_t *cbuf)
{
    size_t size = cbuf->max;

    if (!cbuf->full)
    {
        if (cbuf->head >= cbuf->tail)
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

size_t circular_buf_capacity(cbuf_handle_t cbuf)
{
    assert(cbuf);

    return cbuf->max;
}



int circular_buf_get(struct circular_buf_t * cbuf, char * data)
{
    int r = -1;

    if(!circular_buf_empty(cbuf))
    {
        strcpy(data,cbuf->buffer[cbuf->tail]);
        retreat_pointer(cbuf);

        r = 0;
    }

    return r;
}

bool circular_buf_empty(struct circular_buf_t *cbuf)
{
    return (!cbuf->full && (cbuf->head == cbuf->tail));
}

bool circular_buf_full(struct circular_buf_t *cbuf)
{
    return cbuf->full;
}
