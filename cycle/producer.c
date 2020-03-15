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
#include "../shared/shared.h"
#include "../circular_buffer/circular_buffer.h"
#include "../circular_buffer/circular_buffer.c"

//#define EXAMPLE_BUFFER_SIZE 10

const char * NAME_MEMORY_SUSPEND = "SHARED_MEMORY_SUSPEND";
int SHAREDM_FILEDESCRIPTOR_SUSPEND;   //shared memory file discriptor for suspend process
int * SUSPEND; //If true suspend the process

int *producers;
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
  char *name = "shared_memory";
  const char *p_mem = "p_mem";
  const char *sema1 = "fill";
  const char *sema2 = "avail";
  const char *sema3 = "mutex";
  double timeBlocked = 0;
  double mediumConstant = 0.2;
  if(argc == 1){
    printf("A default value of shared_memory was set for the name of the buffer\n");
    printf("A default value of 0.2 was set for exponential distribution\n");
  }else if (argc == 2) {
    printf("A default value of 0.2 was set for exponential distribution\n");
    printf("%s",argv[1]);
    name = argv[1];
  } else {
    name = argv[1];
    char * end;
    mediumConstant = strtod (argv[2], & end);
    if (end == argv[2]) {
      printf ("Second parameter is not a valid number.\n");
      exit(0);
    }
  }
  const int bufsize = 80;
  int hours, minutes, seconds, day, month, year;
  time_t begin = time(NULL);
  int shm_fd; //shared memory file discriptor
  int p_shm_fd;
  struct circular_buf_t *shared_mem_ptr;
  int val;
  sem_t *fill, *avail, *mutex;
  /* make * shelf shared between processes*/
  //create the shared memory segment
  shm_fd = shm_open(name, O_RDWR, 0666);
  p_shm_fd = shm_open(producers_mem_name, O_RDWR, 0666);
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

  //create the shared memory segment
  SHAREDM_FILEDESCRIPTOR_SUSPEND = shm_open(NAME_MEMORY_SUSPEND, O_CREAT | O_RDWR, 0666);
  //configure the size of the shared memory segment
  ftruncate(SHAREDM_FILEDESCRIPTOR_SUSPEND,sizeof(int));
  //map the shared memory segment in process address space
  SUSPEND = mmap(0,sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, SHAREDM_FILEDESCRIPTOR_SUSPEND, 0);

  //print_buffer_status(shared_mem_ptr);
  printf("\nProducer: I have started producing messages.\n");
  (* producers)++;
  *SUSPEND = 1;
  while(*SUSPEND > 0){
    sem_getvalue(avail, &val);
    //printf(" (sem_wait) avail semaphore % d \n", val);
    sem_getvalue(fill, &val);
    //printf(" (sem_wait) fill semaphore % d \n", val);
    time_t beginSemaphore = time(NULL);
    sem_wait(avail);
    int sleepTime = ran_expo(mediumConstant);
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
    printf("Inserting message in buffer at position %i. Producers: %i\n",shared_mem_ptr->head, *producers);
    circular_buf_put(shared_mem_ptr, s);
    totalMessages++;
    //print_buffer_status(shared_mem_ptr);
    sem_post(mutex);
    sem_post(fill);
    time_t endSemaphore = time(NULL);
    timeBlocked = timeBlocked + (beginSemaphore - endSemaphore) - sleepTime;
  }
  sem_wait(mutex);
  (* producers)--;
  sem_post(mutex);
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
  time_t end = time(NULL);
  // calculate elapsed time by finding difference (end - begin)
  printf("Time elpased is %d seconds\n", (end - begin));
  printf("Total messages created: %i\n", totalMessages);
  printf("Total time blocked by semaphores in seconds: %d\n", totalMessages);
  return 0;
}
