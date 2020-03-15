#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/shm.h>
#include "circular_buffer.h"


// The definition of our circular buffer structure is hidden from the user
struct circular_buf_t
{
    char **buffer;
    size_t head;
    size_t tail;
    size_t max; //of the buffer
    bool full;
};

#pragma mark - Private Functions -

cbuf_handle_t circular_buf_init(cbuf_handle_t cbuf, size_t size)
{
	//cbuf_handle_t cbuf //= malloc(sizeof(circular_buf_t));

	assert(cbuf);
  //size_t cb = sizeof(char *) * size;

  /* cbuf->buffer = (char **)malloc(sizeof(char *) * size);
  for(int i = 0; i < size; i++){
    cbuf->buffer[i] = (char *)malloc(sizeof(char) * 256);
  } */

  cbuf->buffer = (char **)mmap(cbuf->buffer, sizeof(char *) * size, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
  for(int i = 0; i < size; i++){
    cbuf->buffer[i] = (char *)mmap(cbuf->buffer[i], sizeof(char) * 256, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
  }

  if (cbuf->buffer == MAP_FAILED)
        exit(EXIT_FAILURE);

	cbuf->max = size;
	circular_buf_reset(cbuf);

	assert(circular_buf_empty(cbuf));

  printf("sizeof(cbuf)=%zu\n",sizeof(cbuf));

	return cbuf;
}

void advance_pointer(cbuf_handle_t cbuf)
{

    if (cbuf->full)
    {
        cbuf->tail = (cbuf->tail + 1) % cbuf->max;
    }

    cbuf->head = (cbuf->head + 1) % cbuf->max;

    // We mark full because we will advance tail on the next time around
    cbuf->full = (cbuf->head == cbuf->tail);
}

int circular_buf_put(cbuf_handle_t cbuf, char *data)
{
    int r = -1;

    if (!circular_buf_full(cbuf))
    {
        //cbuf->buffer[cbuf->head] = (char *)malloc(sizeof(char) * 256);
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
    for(int i = 0; i < cbuf->max; i++){
      free(cbuf->buffer[i]);
    }
    free(cbuf->buffer);
    free(cbuf);
}

void print_buffer_status(cbuf_handle_t cbuf)
{
    printf("Full: %d, empty: %d, size: %zu\n",
           circular_buf_full(cbuf),
           circular_buf_empty(cbuf),
           circular_buf_size(cbuf));
}

void circular_buf_reset(cbuf_handle_t cbuf)
{
    cbuf->head = 0;
    cbuf->tail = 0;
    cbuf->full = false;
}

size_t circular_buf_size(cbuf_handle_t cbuf)
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



int circular_buf_get(cbuf_handle_t cbuf, char * data)
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

bool circular_buf_empty(cbuf_handle_t cbuf)
{
    return (!cbuf->full && (cbuf->head == cbuf->tail));
}

bool circular_buf_full(cbuf_handle_t cbuf)
{
    return cbuf->full;
}
