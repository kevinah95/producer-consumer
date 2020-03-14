#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#define EXAMPLE_BUFFER_SIZE 10
static const char * name = "shared_memory";
static const char * p_mem = "p_mem";
static const char * c_mem = "c_mem";
static const char * sema1= "fill";
static const char * sema2= "avail";
static const char * sema3= "mutex";
static int shm_fd;   //shared memory file discriptor
static int p_shm_fd;
static int c_shm_fd;
static struct circular_buf_t *shared_mem_ptr;
static int * producers;
static int val;
static sem_t * fill, * avail, * mutex;