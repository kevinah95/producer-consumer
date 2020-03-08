/*waiter.c*/
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <semaphore.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>


#define EXAMPLE_BUFFER_SIZE 10

struct circular_buf_t
{
  char buffer[EXAMPLE_BUFFER_SIZE][256];
  size_t head;
  size_t tail;
  size_t max; //of the buffer
  bool full;
};

bool circular_buf_empty(struct circular_buf_t *cbuf)
{
  return (!cbuf->full && (cbuf->head == cbuf->tail));
}

bool circular_buf_full(struct circular_buf_t *cbuf)
{
  return cbuf->full;
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

void retreat_pointer(struct circular_buf_t * cbuf)
{

	cbuf->full = false;
	cbuf->tail = (cbuf->tail + 1) % cbuf->max;
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


void print_buffer_status(struct circular_buf_t *cbuf)
{
  printf("Full: %d, empty: %d, size: %zu\n",
         circular_buf_full(cbuf),
         circular_buf_empty(cbuf),
         circular_buf_size(cbuf));
}

int main()
{
    const char * name = "shared_memory";
    const char * p_mem = "p_mem";
    const char * sema1= "fill";
    const char * sema2= "avail";
    const char * sema3="mutex";
    int shm_fd; //file descriptor of
    int p_shm_fd; 
    struct circular_buf_t *shared_mem_ptr;
    int * producers;
    sem_t * fill, * avail, * mutex;
    /* open the shared memory segment */
    shm_fd = shm_open(name, O_RDWR, 0666);
    p_shm_fd = shm_open(p_mem, O_RDWR, 0666);

    ftruncate(shm_fd, sizeof(struct circular_buf_t));
    ftruncate(p_shm_fd, sizeof(int));
    
    /* now map the shared memory segment in the address space of the process*/
    shared_mem_ptr = mmap(0, sizeof(struct circular_buf_t), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    producers = mmap(0,sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, p_shm_fd, 0);
    /* open semaphores*/
    fill = sem_open(sema1, O_RDWR);
    avail = sem_open(sema2, O_RDWR);
    mutex = sem_open(sema3, O_RDWR);

    (* producers)++;
    printf("there are %d Producers now\n", * producers);
    while(1){
        sem_wait(fill);
        sleep(rand()%2+1);
        sem_wait(mutex);
        char data[256];
		    circular_buf_get(shared_mem_ptr, &data);
		    printf("%s ", data);
        print_buffer_status(shared_mem_ptr);
        sem_post(mutex);
        sem_post(avail);
    }

    /* sem_close(fill);
    sem_close(avail);
    sem_close(mutex);
    sem_unlink(sema1);
    sem_unlink(sema2);
    sem_unlink(sema3);

    munmap(shelf, sizeof(int));
    close(shm_fd);
    shm_unlink(name);
    munmap(producers, sizeof(int));
    close(p_shm_fd);
    shm_unlink(p_mem); */
    return 0;
}