#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include "../circular_buffer/circular_buffer.h"

static const char * producers_mem_name = "producers_mem_name";
static const char * consumers_mem_name = "consumers_mem_name";
static const char * fill_sem_name= "fill";
static const char * avail_sem_name= "avail";
static const char * mutex_sem_name= "mutex";
static int buffer_shm_fd;   //shared memory file discriptor
static int producers_shm_fd;
static int consumers_shm_fd;
static cbuf_handle_t buffer_mem_ptr;
static int * producers_mem_ptr;
static int * consumers_mem_ptr;
static sem_t * fill_sem, * avail_sem, * mutex_sem;
