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

#define EXAMPLE_BUFFER_SIZE 10

const char * NAME_MEMORY_SUSPEND = "SHARED_MEMORY_SUSPEND";
int SHAREDM_FILEDESCRIPTOR_SUSPEND;   //shared memory file discriptor for suspend process
int * SUSPEND; //If true suspend the process

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

void advance_pointer(struct circular_buf_t *cbuf)
{

  if (cbuf->full)
  {
    cbuf->tail = (cbuf->tail + 1) % cbuf->max;
  }

  cbuf->head = (cbuf->head + 1) % cbuf->max;

  // We mark full because we will advance tail on the next time around
  cbuf->full = (cbuf->head == cbuf->tail);
}

int circular_buf_put(struct circular_buf_t *cbuf, char *data)
{
  int r = -1;

  if (!circular_buf_full(cbuf))
  {
    strcpy(cbuf->buffer[cbuf->head], data);
    advance_pointer(cbuf);
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

int getRandomNumber(){
  int lower = 0, upper = 4; 
  // Use current time as  
  // seed for random generator 
  srand(time(0)); 
  int num = (rand() % (upper - lower + 1)) + lower; 
  return num;
}

int main()
{
  const char *name = "shared_memory";
  const char *p_mem = "p_mem";
  const char *sema1 = "fill";
  const char *sema2 = "avail";
  const char *sema3 = "mutex";
  const int bufsize = 80;
  int hours, minutes, seconds, day, month, year;
  time_t begin = time(NULL);
  int shm_fd; //shared memory file discriptor
  int p_shm_fd;
  struct circular_buf_t *shared_mem_ptr;
  int *producers;
  int val;
  sem_t *fill, *avail, *mutex;
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

  //create the shared memory segment
  SHAREDM_FILEDESCRIPTOR_SUSPEND = shm_open(NAME_MEMORY_SUSPEND, O_CREAT | O_RDWR, 0666);
  //configure the size of the shared memory segment
  ftruncate(SHAREDM_FILEDESCRIPTOR_SUSPEND,sizeof(int));
  //map the shared memory segment in process address space
  SUSPEND = mmap(0,sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, SHAREDM_FILEDESCRIPTOR_SUSPEND, 0);

  //print_buffer_status(shared_mem_ptr);
  printf("\nProducer: I have started producing messages.\n");
  *SUSPEND = 1;
  while(*SUSPEND > 0){
    sem_getvalue(avail, &val);
    printf(" (sem_wait) avail semaphore % d \n", val);
    sem_getvalue(fill, &val);
    printf(" (sem_wait) fill semaphore % d \n", val);
    sem_wait(avail);
    sleep(5);
    sem_getvalue(mutex, &val);
    printf(" (sem_wait)semaphore mutex % d \n", val);
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
    circular_buf_put(shared_mem_ptr, s);
    //print_buffer_status(shared_mem_ptr);
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
  time_t end = time(NULL);
  // calculate elapsed time by finding difference (end - begin)
  printf("Time elpased is %d seconds\n", (end - begin));
  return 0;
}
