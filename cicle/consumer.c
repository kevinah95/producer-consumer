/*waiter.c*/
#include <stdint.h>
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
#include "../shared/shared.h"
#include "../circular_buffer/circular_buffer.h"
#include "../circular_buffer/circular_buffer.c"

int main()
{
    /* open the shared memory segment */
    shm_fd = shm_open(name, O_RDWR, 0666);
    p_shm_fd = shm_open(p_mem, O_RDWR, 0666);
    c_shm_fd = shm_open(c_mem, O_RDWR, 0666);

    ftruncate(shm_fd, sizeof(struct circular_buf_t));
    ftruncate(p_shm_fd, sizeof(int));
    ftruncate(c_shm_fd, sizeof(int));

    
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