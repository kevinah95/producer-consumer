#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

static const char * producers_mem_name = "producers_mem_name";
static const char * total_messages_name = "/total_messages";
static const char * consumers_mem_name = "/CONSUMERS_SHARED_MEMORY";

static const char * fill_sem_name= "fill";
static const char * avail_sem_name= "avail";
static const char * mutex_sem_name= "mutex";

const char * NAME_MEMORY_SUSPEND = "SHARED_MEMORY_SUSPEND";

static int buffer_shm_fd;   //shared memory file discriptor
static int producers_shm_fd;
static int consumers_shm_fd;
static int total_messages_shm_fd;
static int SHAREDM_FILEDESCRIPTOR_SUSPEND;   //shared memory file discriptor for suspend process

static struct circular_buf_t *buffer_mem_ptr;

static int * producers_mem_ptr;
static int * consumers_mem_ptr;
static int * SUSPEND; //If true suspend the process
static int * total_messages; //If true suspend the process

static sem_t * fill_sem, * avail_sem, * mutex_sem;

const char special_message[] = "KILLER_MESSAGE_FROM_FINISHER"; // Special message that indicates the process must finish

static int default_buffer_size = 10;
char default_buffer_name[] = "shared_memory";
