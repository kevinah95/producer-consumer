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
#include <getopt.h>
#include "../circular_buffer/circular_buffer.h"
#include "../circular_buffer/circular_buffer.c"
#include "../shared/shared.h"

int main(int argc, char *argv[])
{
  int opt, val;
  size_t buffer_size;
  char *buffer_name;

  buffer_size = 0;
  while ((opt = getopt(argc, argv, "s:n")) != -1)
  {
    switch (opt)
    {
    case 'n':
      buffer_name = strdup(argv[optind]);
      break;
    case 's':
      buffer_size = atoi(optarg);
      break;
    default:
      fprintf(stderr, "Usage: %s [-n buffer_name] [-s buffer_size]\n",
              argv[0]);
      exit(EXIT_FAILURE);
    }
  }

  if (optind >= argc)
  {
    fprintf(stderr, "Expected argument after options\n\tUsage: %s [-n buffer_name] [-s buffer_size]\n", argv[0]);
    exit(EXIT_FAILURE);
  }

  printf("buffer_name argument = %s\n", buffer_name);
  printf("buffer_size argument = %zu\n", buffer_size);

  //create the shared memory segment
  buffer_shm_fd = shm_open(buffer_name, O_CREAT | O_RDWR, 0666);
  producers_shm_fd = shm_open(producers_mem_name, O_CREAT | O_RDWR, 0666);
  consumers_shm_fd = shm_open(consumers_mem_name, O_CREAT | O_RDWR, 0666);
  SHAREDM_FILEDESCRIPTOR_SUSPEND = shm_open(NAME_MEMORY_SUSPEND, O_CREAT | O_RDWR, 0666);
  total_messages_shm_fd = shm_open(total_messages_name, O_CREAT | O_RDWR, 0666);

  //configure the size of the shared memory segment
  ftruncate(buffer_shm_fd, sizeof(struct circular_buf_t));
  ftruncate(producers_shm_fd, sizeof(int));
  ftruncate(consumers_shm_fd, sizeof(int));
  ftruncate(SHAREDM_FILEDESCRIPTOR_SUSPEND,sizeof(int));
  ftruncate(total_messages_shm_fd,sizeof(int));
  //map the shared memory segment in process address space
  buffer_mem_ptr = mmap(0, sizeof(struct circular_buf_t), PROT_READ | PROT_WRITE, MAP_SHARED, buffer_shm_fd, 0);
  producers_mem_ptr = mmap(0, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, producers_shm_fd, 0);
  consumers_mem_ptr = mmap(0, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, consumers_shm_fd, 0);
  SUSPEND = mmap(0,sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, SHAREDM_FILEDESCRIPTOR_SUSPEND, 0);
  total_messages = mmap(0,sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, total_messages_shm_fd, 0);
  *total_messages = 0;
  *SUSPEND = 1; //Enable creation of messages for producers
  /* creat/open semaphores*/

  fill_sem = sem_open(fill_sem_name, O_RDWR | O_CREAT | O_EXCL, S_IRUSR | S_IWUSR, 0);

  avail_sem = sem_open(avail_sem_name, O_RDWR | O_CREAT | O_EXCL, S_IRUSR | S_IWUSR, buffer_size);

  mutex_sem = sem_open(mutex_sem_name, O_RDWR | O_CREAT | O_EXCL, S_IRUSR | S_IWUSR, 1);

  buffer_mem_ptr->max = buffer_size;
  circular_buf_reset(buffer_mem_ptr);

  sem_getvalue(fill_sem, &val);
  printf(" (sem_wait) fill semaphore % d \n", val);

  sem_getvalue(avail_sem, &val);
  printf(" (sem_wait) avail semaphore % d \n", val);

  sem_getvalue(mutex_sem, &val);
  printf(" (sem_wait) semaphore mutex % d \n", val);

  free(buffer_name);

  exit(EXIT_SUCCESS);
}
