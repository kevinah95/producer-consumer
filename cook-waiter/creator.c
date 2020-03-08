#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <semaphore.h>
#include <stdbool.h> 
#include <unistd.h>

#define EXAMPLE_BUFFER_SIZE 10

struct circular_buf_t {
	char buffer[EXAMPLE_BUFFER_SIZE][256];
	size_t head;
	size_t tail;
	size_t max; //of the buffer
	bool full;
};

void circular_buf_reset(struct circular_buf_t * cbuf)
{
    cbuf->head = 0;
    cbuf->tail = 0;
    cbuf->full = false;
}

int main()
{
    const char * name = "shared_memory";
    const char * p_mem = "p_mem";
    const char * sema1= "fill";
    const char * sema2= "avail";
    const char * sema3= "mutex";
    int shm_fd;   //shared memory file discriptor
    int p_shm_fd; 
    struct circular_buf_t *shared_mem_ptr;
    int * producers; 
    int val;
    sem_t * fill, * avail, * mutex;


    //create the shared memory segment
    shm_fd = shm_open(name, O_CREAT | O_RDWR, 0666);
    p_shm_fd = shm_open(p_mem, O_CREAT | O_RDWR, 0666);
    //configure the size of the shared memory segment
    ftruncate(shm_fd,sizeof (struct circular_buf_t));
    ftruncate(p_shm_fd,sizeof(int));
    //map the shared memory segment in process address space
    shared_mem_ptr = mmap(0,sizeof(struct circular_buf_t), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    producers = mmap(0,sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, p_shm_fd, 0);
    /* creat/open semaphores*/
    //cook post semaphore fill after cooking a pizza
    fill = sem_open(sema1, O_RDWR | O_CREAT | O_EXCL, S_IRUSR | S_IWUSR,0);
    //waiter post semaphore avail after picking up a pizza, at the beginning avail=3
    avail = sem_open(sema2, O_RDWR | O_CREAT | O_EXCL, S_IRUSR | S_IWUSR, EXAMPLE_BUFFER_SIZE);
    //mutex for mutual exclusion of shelf
    mutex = sem_open(sema3,O_RDWR | O_CREAT | O_EXCL, S_IRUSR | S_IWUSR, 1);

    shared_mem_ptr->max = EXAMPLE_BUFFER_SIZE;
    circular_buf_reset(shared_mem_ptr);

    sem_getvalue(fill, &val);
    printf(" (sem_wait) fill semaphore % d \n", val);

    sem_getvalue(avail, &val);
    printf(" (sem_wait) avail semaphore % d \n", val);
    
    sem_getvalue(mutex, &val);
    printf(" (sem_wait) semaphore mutex % d \n", val);
        
    return 0;
}

