#include <stdint.h>
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
#include "../shared/shared.h"
#include "../circular_buffer/circular_buffer.h"
#include "../circular_buffer/circular_buffer.c"

int main()
{
  /* make * shelf shared between processes*/
  //create the shared memory segment
  shm_fd = shm_open(name, O_RDWR, 0666);
  p_shm_fd = shm_open(p_mem, O_RDWR, 0666);
  //configure the size of the shared memory segment
  ftruncate(shm_fd, sizeof(struct circular_buf_t));
  ftruncate(p_shm_fd, sizeof(int));
  //map the shared memory segment in process address space
  shared_mem_ptr = mmap(0, sizeof(struct circular_buf_t), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
  producers = mmap(0, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, p_shm_fd, 0);
  /* creat/open semaphores*/
  //cook post semaphore fill after cooking a pizza
  fill = sem_open(sema1, O_RDWR);
  avail = sem_open(sema2, O_RDWR);
  mutex = sem_open(sema3, O_RDWR);
  print_buffer_status(shared_mem_ptr);
  printf("\nCook: I have started cooking pizza.\n");
  while (1)
  {
    sem_getvalue(avail, &val);
    printf(" (sem_wait) avail semaphore % d \n", val);
    sem_getvalue(fill, &val);
    printf(" (sem_wait) fill semaphore % d \n", val);
    sem_wait(avail);
    sleep(5);
    sem_getvalue(mutex, &val);
    printf(" (sem_wait)semaphore mutex % d \n", val);
    sem_wait(mutex);
    char *s;
    sprintf(s, "pre_%zu_suff", shared_mem_ptr->head);
    circular_buf_put(shared_mem_ptr, s);
    print_buffer_status(shared_mem_ptr);
    sem_post(mutex);
    sem_post(fill);
  }
  /* close and unlink semaphores*/
  /* sem_close(fill);
    sem_close(avail);
    sem_close(mutex);
    sem_unlink(sema1);
    sem_unlink(sema2);
    sem_unlink(sema3);

      munmap(shelf, sizeof(int));
    close(shm_fd);
    shm_unlink(name); */
  return 0;
}