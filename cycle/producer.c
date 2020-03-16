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
#include <math.h> // math for exponential function
#include <time.h> // time for seeding
#include <getopt.h>
#include "../shared/shared.h"
#include "../circular_buffer/circular_buffer.h"
#include "../circular_buffer/circular_buffer.c"

int *is_not_suspended;
int totalMessages = 0;

int getRandomNumber()
{
  int lower = 0, upper = 4;
  // Use current time as
  // seed for random generator
  srand(time(0));
  int num = (rand() % (upper - lower + 1)) + lower;
  return num;
}

double ran_expo(double lambda)
{
  double u;
  u = rand() / (RAND_MAX + 1.0);
  return -log(1 - u) / lambda;
}

int main(int argc, char *argv[])
{
  char *buffer_name = "shared_memory";
  double timeBlocked = 0;
  double timeWaiting = 0;
  char *endDecimalConvert;
  double mediumConstant = 0.2;
  int opt;
  while ((opt = getopt(argc, argv, "m:n")) != -1)
  {
    switch (opt)
    {
    case 'n':
      buffer_name = strdup(argv[optind]);
      break;
    case 'm':
      mediumConstant = strtod(optarg, &endDecimalConvert);
      if (endDecimalConvert == optarg)
      {
        printf("-m parameter does not has a valid number.\n");
        exit(0);
      }
      break;
    default:
      fprintf(stderr, "Usage: %s [-n buffer_name] [-m medium]\n",
              argv[0]);
      exit(EXIT_FAILURE);
    }
  }

  if (optind >= argc)
  {
    fprintf(stderr, "Expected argument after options\n\tUsage: %s [-n buffer_name] [-m medium]\n", argv[0]);
    exit(EXIT_FAILURE);
  }

  int hours, minutes, seconds, day, month, year;

  int val;
  char buffer_prime_name[256];
  strcpy(buffer_prime_name, buffer_name);
  strcat(buffer_prime_name, "_prime");
  printf("%s",buffer_prime_name);
  /* make * shelf shared between processes*/
  //create the shared memory segment

  buffer_shm_fd = shm_open(buffer_name, O_RDWR, 0666);
  producers_shm_fd = shm_open(producers_mem_name, O_RDWR, 0666);
  consumers_shm_fd = shm_open(consumers_mem_name, O_RDWR, 0666);
  SHAREDM_FILEDESCRIPTOR_SUSPEND = shm_open(NAME_MEMORY_SUSPEND, O_RDWR, 0666);
  //configure the size of the shared memory segment
  ftruncate(buffer_shm_fd, sizeof(struct circular_buf_t));

  ftruncate(producers_shm_fd, sizeof(int));
  ftruncate(consumers_shm_fd, sizeof(int));
  ftruncate(SHAREDM_FILEDESCRIPTOR_SUSPEND, sizeof(int));
  //map the shared memory segment in process address space
  buffer_mem_ptr = mmap(0, sizeof(struct circular_buf_t), PROT_READ | PROT_WRITE, MAP_SHARED, buffer_shm_fd, 0);

  buffer_prime_shm_fd = shm_open(buffer_prime_name, O_CREAT | O_RDWR, 0666);
  ftruncate(buffer_prime_shm_fd, buffer_mem_ptr->max * 256);
  buffer_mem_ptr->buffer = mmap(NULL, buffer_mem_ptr->max * 256, PROT_READ | PROT_WRITE, MAP_SHARED, buffer_prime_shm_fd, 0);

  producers_mem_ptr = mmap(0, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, producers_shm_fd, 0);
  consumers_mem_ptr = mmap(0, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, consumers_shm_fd, 0);
  is_not_suspended = mmap(0, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, SHAREDM_FILEDESCRIPTOR_SUSPEND, 0);
  /* creat/open semaphores*/
  //cook post semaphore fill after cooking a pizza
  fill_sem = sem_open(fill_sem_name, O_RDWR);
  avail_sem = sem_open(avail_sem_name, O_RDWR);
  mutex_sem = sem_open(mutex_sem_name, O_RDWR);

  //print_buffer_status(buffer_mem_ptr);
  printf("\nProducer: I have started producing messages.\n");
  (*producers_mem_ptr)++;
  while (*is_not_suspended > 0)
  {
    sem_getvalue(avail_sem, &val);
    //printf(" (sem_wait) avail semaphore % d \n", val);
    sem_getvalue(fill_sem, &val);
    //printf(" (sem_wait) fill semaphore % d \n", val);
    time_t beginSemaphore = time(NULL);
    sem_wait(avail_sem);
    time_t endSemaphore = time(NULL);
    timeBlocked = timeBlocked + (beginSemaphore - endSemaphore);
    int sleepTime = ran_expo(mediumConstant);
    timeWaiting = timeWaiting + sleepTime;
    sleep(sleepTime);

    sem_getvalue(mutex_sem, &val);
    //printf(" (sem_wait)semaphore mutex % d \n", val);
    sem_wait(mutex_sem);
    // time_t is arithmetic time type
    time_t now;

    // Obtain current time
    // time() returns the current time of the system as a time_t value
    time(&now);

    // localtime converts a time_t value to calendar time and
    // returns a pointer to a tm structure with its members
    // filled with the corresponding values
    struct tm *local = localtime(&now);

    hours = local->tm_hour;  // get hours since midnight (0-23)
    minutes = local->tm_min; // get minutes passed after the hour (0-59)
    seconds = local->tm_sec; // get seconds passed after minute (0-59)

    day = local->tm_mday;         // get day of month (1 to 31)
    month = local->tm_mon + 1;    // get month of year (0 to 11)
    year = local->tm_year + 1900; // get year since 1900

    char s[256] = "";
    int charcheck = snprintf(s, 256 - 1, "%d-Date: %i/%i/%i Time: %i:%i:%i-%i\n", getpid(), year, month, day, hours, minutes, seconds, getRandomNumber());
    printf("Inserting message in buffer at position %zu. Producers: %i. Consumers:%i\n", buffer_mem_ptr->head, *producers_mem_ptr, *consumers_mem_ptr);
    circular_buf_put(buffer_mem_ptr, s);
    totalMessages++;
    //print_buffer_status(buffer_mem_ptr);
    sem_post(mutex_sem);
    sem_post(fill_sem);
  }
  sem_wait(mutex_sem);
  (*producers_mem_ptr)--;
  sem_post(mutex_sem);
  /* close and unlink semaphores*/

  sem_close(fill_sem);
  sem_close(avail_sem);
  sem_close(mutex_sem);
  sem_unlink(fill_sem_name);
  sem_unlink(avail_sem_name);
  sem_unlink(mutex_sem_name);

  // calculate elapsed time by finding difference (end - begin)
  printf("Producer %i terminated.\n", getpid());
  printf("Total messages created: %i\n", totalMessages);
  printf("Total time waiting in seconds: %d\n", timeWaiting);
  printf("Total time blocked by semaphores in seconds: %d\n", timeBlocked);
  return 0;
}
