#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h> // math for exponential function
#include <time.h> // time for seeding
#include <pthread.h> // include pthread functions and structures
#include <semaphore.h> // include semaphores
#include <unistd.h> // include sleep
#include <fcntl.h>
#include <sys/shm.h>
#include <sys/mman.h>
#include <getopt.h>
#include "../shared/shared.h"
#include "../circular_buffer/circular_buffer.h"
#include "../circular_buffer/circular_buffer.c"

int main(int argc, char *argv[]) {

  int opt, val;
  size_t buffer_size;
  char *buffer_name;
  while ((opt = getopt(argc, argv, "s:n")) != -1)
  {
    switch (opt)
    {
    case 'n':
      buffer_name = strdup(argv[optind]);
      break;
    case 's':
      break;
    default:
      fprintf(stderr, "Usage: %s [-n buffer_name]\n",
              argv[0]);
      exit(EXIT_FAILURE);
    }
  }

  if (optind >= argc)
  {
    fprintf(stderr, "Expected argument after options\n\tUsage: %s [-n buffer_name]\n", argv[0]);
    exit(EXIT_FAILURE);
  }

  printf("Killer...\n");
  //create the shared memory segment
  producers_shm_fd = shm_open(producers_mem_name,O_RDWR, 0666);
  consumers_shm_fd = shm_open(consumers_mem_name, O_RDWR, 0666);
  SHAREDM_FILEDESCRIPTOR_SUSPEND = shm_open(NAME_MEMORY_SUSPEND, O_RDWR, 0666);
  total_messages_shm_fd = shm_open(total_messages_name, O_RDWR, 0666);

  //configure the size of the shared memory segment
  ftruncate(producers_shm_fd, sizeof(int));
  ftruncate(consumers_shm_fd, sizeof(int));
  ftruncate(SHAREDM_FILEDESCRIPTOR_SUSPEND,sizeof(int));
  ftruncate(total_messages_shm_fd,sizeof(int));
  //map the shared memory segment in process address space
  producers_mem_ptr = mmap(0, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, producers_shm_fd, 0);
  consumers_mem_ptr = mmap(0, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, consumers_shm_fd, 0);
  SUSPEND = mmap(0,sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, SHAREDM_FILEDESCRIPTOR_SUSPEND, 0);
  total_messages = mmap(0,sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, total_messages_shm_fd, 0);
  int totalProducers = *producers_mem_ptr;
  int totalConsumers = *consumers_mem_ptr;
  int shm_fd; //shared memory file discriptor
  struct circular_buf_t *shared_mem_ptr;
  sem_t *fill, *avail, *mutex;
  const char *p_mem = "p_mem";
  const char *sema1 = "fill";
  const char *sema2 = "avail";
  const char *sema3 = "mutex";
  /* make * shelf shared between processes*/
  //create the shared memory segment
  shm_fd = shm_open(buffer_name, O_RDWR, 0666);
  //configure the size of the shared memory segment
  ftruncate(shm_fd, sizeof(struct circular_buf_t));

  //map the shared memory segment in process address space
  shared_mem_ptr = mmap(0, sizeof(struct circular_buf_t), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);

  //cook post semaphore fill after cooking a pizza
  fill = sem_open(sema1, O_RDWR);
  avail = sem_open(sema2, O_RDWR);
  mutex = sem_open(sema3, O_RDWR);
  *SUSPEND = 0; //Suspend producers
  printf("Producers suspended...\n\n");
  while(*consumers_mem_ptr > 0){
    printf("%i consumers remaining...\n",*consumers_mem_ptr);
    sem_getvalue(avail, &val);
    sem_getvalue(fill, &val);
    //sem_wait(avail);

    sem_getvalue(mutex, &val);
    //printf(" (sem_wait)semaphore mutex % d \n", val);
    sem_wait(mutex);

    char s[256] ="";
    int charcheck = snprintf(s, 256 - 1, "KILLER_MESSAGE_FROM_FINISHER");
    circular_buf_put(shared_mem_ptr, s);
    //print_buffer_status(shared_mem_ptr);
    sem_post(mutex);
    //sem_post(fill);
  }
  sleep(2);
  sem_post(avail);
  sleep(3);
  printf("\nTotal producers created: %i\n", totalProducers);
  printf("Total consumers killed: %i\n", totalConsumers);
  printf("Total messages created: %i\n", *total_messages);

  munmap(SUSPEND, sizeof(int));
  close(SHAREDM_FILEDESCRIPTOR_SUSPEND);

  munmap(producers_mem_ptr, sizeof(int));
  close(producers_shm_fd);

  munmap(consumers_mem_ptr, sizeof(int));
  close(consumers_shm_fd);


  exit(0);
}
