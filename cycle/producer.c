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
int *producers;
int *consumers;
int totalMessages = 0;

int getRandomNumber(){
  int lower = 0, upper = 4; 
  // Use current time as  
  // seed for random generator 
  srand(time(0)); 
  int num = (rand() % (upper - lower + 1)) + lower; 
  return num;
}

double ran_expo(double lambda) {
    double u;
    u = rand() / (RAND_MAX + 1.0);
    return -log(1- u) / lambda;
}

int main(int argc, char *argv[])
{
  char *buffer_name = "shared_memory";
  const char *p_mem = "p_mem";
  const char *sema1 = "fill";
  const char *sema2 = "avail";
  const char *sema3 = "mutex";
  double timeBlocked = 0;
  double timeWaiting = 0;
  char * endDecimalConvert;
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
      mediumConstant = strtod (optarg, & endDecimalConvert);
      if (endDecimalConvert == optarg) {
        printf ("-m parameter does not has a valid number.\n");
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
  int shm_fd; //shared memory file discriptor
  int p_shm_fd;
  int consumer_shm_fd;
  struct circular_buf_t *shared_mem_ptr;
  int val;
  sem_t *fill, *avail, *mutex;
  /* make * shelf shared between processes*/
  //create the shared memory segment
  shm_fd = shm_open(buffer_name, O_RDWR, 0666);
  p_shm_fd = shm_open(producers_mem_name, O_RDWR, 0666);
  consumer_shm_fd = shm_open(consumers_mem_name, O_RDWR, 0666);
  SHAREDM_FILEDESCRIPTOR_SUSPEND = shm_open(NAME_MEMORY_SUSPEND, O_RDWR, 0666);
  total_messages_shm_fd = shm_open(total_messages_name, O_RDWR, 0666);

  //configure the size of the shared memory segment
  ftruncate(shm_fd, sizeof(struct circular_buf_t));
  ftruncate(p_shm_fd, sizeof(int));
  ftruncate(consumer_shm_fd, sizeof(int));
  ftruncate(SHAREDM_FILEDESCRIPTOR_SUSPEND,sizeof(int));
  ftruncate(total_messages_shm_fd,sizeof(int));
  //map the shared memory segment in process address space
  shared_mem_ptr = mmap(0, sizeof(struct circular_buf_t), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
  producers = mmap(0, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, p_shm_fd, 0);
  consumers = mmap(0, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, consumer_shm_fd, 0);
  total_messages = mmap(0,sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, total_messages_shm_fd, 0);
  is_not_suspended = mmap(0,sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, SHAREDM_FILEDESCRIPTOR_SUSPEND, 0);
  /* creat/open semaphores*/
  //cook post semaphore fill after cooking a pizza
  fill = sem_open(sema1, O_RDWR);
  avail = sem_open(sema2, O_RDWR);
  mutex = sem_open(sema3, O_RDWR);
  //print_buffer_status(shared_mem_ptr);
  printf("\nProducer: I have started producing messages.\n");
  (* producers)++;
  while(*is_not_suspended > 0){
    sem_getvalue(avail, &val);
    //printf(" (sem_wait) avail semaphore % d \n", val);
    sem_getvalue(fill, &val);
    //printf(" (sem_wait) fill semaphore % d \n", val);
    time_t beginSemaphore = time(NULL);
    sem_wait(avail);
    time_t endSemaphore = time(NULL);
    timeBlocked = timeBlocked + (endSemaphore - beginSemaphore);
    int sleepTime = ran_expo(mediumConstant);
    timeWaiting = timeWaiting + sleepTime;
    sleep(sleepTime);

    sem_getvalue(mutex, &val);
    //printf(" (sem_wait)semaphore mutex % d \n", val);
    sem_wait(mutex);
    // time_t is arithmetic time type
    time_t now;
    
    // Obtain current time
    // time() returns the current time of the system as a time_t value
    time(&now);

    // localtime converts a time_t value to calendar time and 
    // returns a pointer to a tm structure with its members 
    // filled with the corresponding values
    struct tm *local = localtime(&now);

    hours = local->tm_hour;      	// get hours since midnight (0-23)
    minutes = local->tm_min;     	// get minutes passed after the hour (0-59)
    seconds = local->tm_sec;     	// get seconds passed after minute (0-59)

    day = local->tm_mday;        	// get day of month (1 to 31)
    month = local->tm_mon + 1;   	// get month of year (0 to 11)
    year = local->tm_year + 1900;	// get year since 1900

    char s[256] ="";
    int charcheck = snprintf(s, 256 - 1, "%d-Date: %i/%i/%i Time: %i:%i:%i-%i\n",getpid(),year,month,day,hours,minutes,seconds,getRandomNumber());
    printf("Inserting message in buffer at position %ld. Producers: %i. Consumers:%i\n",shared_mem_ptr->head, *producers, *consumers);
    circular_buf_put(shared_mem_ptr, s);
    totalMessages++;
    *total_messages = *total_messages + 1;
    //print_buffer_status(shared_mem_ptr);
    sem_post(mutex);
    sem_post(fill);
  }
  (* producers)--;

  // calculate elapsed time by finding difference (end - begin)
  printf("Producer %i terminated.\n", getpid());
  printf("Total messages created: %i\n", totalMessages);
  printf("Total time waiting in seconds: %f\n", timeWaiting);
  printf("Total time blocked by semaphores in seconds: %f\n", timeBlocked);
  return 0;
}
