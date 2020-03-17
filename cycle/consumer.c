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
#include <math.h>
#include <time.h>
#include <getopt.h>
#include "../shared/shared.h"
#include "../circular_buffer/circular_buffer.h"
#include "../circular_buffer/circular_buffer.c"

//#define CONSUMERS_SHARED_STORAGE_ID "/CONSUMERS_SHARED_MEMORY"
//#define PRODUCERS_SHARED_STORAGE_ID "/p_mem"

//const char special_message[] = "KILLER_MESSAGE_FROM_FINISHER"; // Special message that indicates the process must finisha

const char * prod_counter_mutex= "/prod_counter_mutex";
const char * con_counter_mutex = "/con_counter_mutex";

sem_t * sem_con_producer_mutex; // Semaphore to control the producer counter of shared memory
sem_t * sem_con_counter_mutex; // Semaphore to control the consumer counter of shared memory

int * consumer_counter; // Represents the number of consumer processes alive
int * producer_counter; // Represents the number of producer processes alive

int c_shm_fd; // Shared memory file discriptor
int p_shm_fd;

int counter_read_messages = 0; // Counts the read messages

time_t begin;

double meanConstant = 0.2;

char * buffer_name = default_buffer_name;
struct circular_buf_t * shared_circular_buffer;
sem_t * fill, * avail, * mutex;

double timeBlocked = 0;
double total_time_sleeping = 0;

/**
* Initializes mutex semaphores to control the counters of shared memory
*/
void initialize_semaphores() {
  sem_con_producer_mutex = sem_open(prod_counter_mutex, O_CREAT, 0644, 1);
  sem_con_counter_mutex = sem_open(con_counter_mutex, O_CREAT, 0644, 1);
  //printf("Semaphores have been initialized\n");
}

/**
* Close and unlink used semaphores
*/
void close_semaphores() {
  sem_close(sem_con_producer_mutex);
  sem_close(sem_con_counter_mutex);
  sem_unlink(prod_counter_mutex);
  sem_unlink(con_counter_mutex);
}

/**
* Close and unlink used shared memory
*/
void close_shared_memory() {
  munmap(consumer_counter, sizeof(int));
  munmap(producer_counter, sizeof(int));
  close(c_shm_fd);
  close(p_shm_fd);
  shm_unlink(consumers_mem_name);
  shm_unlink(producers_mem_name);
}

/**
* Random number following an exponential distribution determined
* by a lambda which represents a mean value
*/
double ran_expo(double lambda) {
    double u;
    u = rand() / (RAND_MAX + 1.0);
    return -log(1- u) / lambda;
}

/**
* Calculates mod5 of a particular integer
*/
int calculate_mod(int pid) {
  return pid %5;
}

/**
* Determines if the read message is a stop-process message
*/
bool is_killer_message(char message[]) {
  char *token = strtok(message, "-");

  if(strcmp(message, special_message) == 0) {
    return true;
  }
  int index = 0;

  while(token != NULL) {
    if(index == 2) {
      return atoi(token) == calculate_mod(getpid());
    }
    token = strtok(NULL, "-");
    index = index + 1;
  }
  return false;
}

/**
* Read from shared memory the counter of consumers and increments or decrements its value by 1
* the action depends on isIncrement, if true it increments, otherwise decrements
*/
int read_and_modify_consumer_counter(bool isIncrement) {
  pid_t pid;
  pid = getpid();

  // Get shared memory file descriptor (not a file)
  c_shm_fd = shm_open(consumers_mem_name, O_RDWR, S_IRUSR | S_IWUSR);
  if (c_shm_fd == -1)
  {
    perror("open");
    return 10;
  }

  // Configure the size of the shared memory segment
  ftruncate(c_shm_fd, sizeof(int));

  // Map shared memory to process address space
  consumer_counter = mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, c_shm_fd, 0);
  if (consumer_counter == MAP_FAILED)
  {
    perror("mmap");
    return 30;
  }

  //printf("PID %d: Read from shared memory: \"%d\"\n", pid, *consumer_counter);

  sem_wait(sem_con_counter_mutex);
  if(isIncrement) {
    (* consumer_counter)++;
  } else {
    (* consumer_counter)--;
  }
  sem_post(sem_con_counter_mutex);
  //printf("consumer_counter has been increased/decreased\n");

  return 0;
}

/**
* Read from shared memory the counter of producers
*/
int read_producer_counter() {
  pid_t pid;
  pid = getpid();

  // Get shared memory file descriptor (not a file)
  p_shm_fd = shm_open(producers_mem_name, O_RDWR, S_IRUSR | S_IWUSR);
  if (p_shm_fd == -1)
  {
    perror("open");
    return 10;
  }

  // Configure the size of the shared memory segment
  ftruncate(p_shm_fd, sizeof(int));

  // Map shared memory to process address space
  producer_counter = mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, p_shm_fd, 0);
  if (producer_counter == MAP_FAILED)
  {
    perror("mmap");
    return 30;
  }

  return 0;
}

void stop_consumer() {
  read_and_modify_consumer_counter(false);
  printf("***************************************************\n");
  printf("Consumer %i: this consumer has been stopped\n", getpid());
  //printf("Consumer %i: this consumer remained alive %ld second(s)\n", getpid(), (time(NULL) - begin));
  printf("Consumer %i: this consumer was blocked %f second(s)\n", getpid(), timeBlocked);
  printf("Consumer %i: this consumer was slept %f second(s)\n", getpid(), total_time_sleeping);
  printf("Consumer %i: this consumer read %d message(s)\n", getpid(), counter_read_messages);
  printf("Consumer %i: there is/are %i consumer(s) alive\n", getpid(), * consumer_counter);
  read_producer_counter();
  printf("Consumer %i: there is/are %i producer(s) alive\n", getpid(), * producer_counter);
  //close_semaphores();
  //close_shared_memory();
  sleep(1);
  exit(0);
}

void initialize_buffer_semaphores() {
  //const char * name = "buffer";
  //const char * sema1= "fill";
  //const char * sema2= "avail";
  //const char * sema3= "mutex";

  int buffer_shm_fd; // Buffer (shared memory) file discriptor

  // Open the shared memory segment
  buffer_shm_fd = shm_open(buffer_name, O_RDWR, 0666);
  ftruncate(buffer_shm_fd, sizeof(struct circular_buf_t));

  // Now map the shared memory segment of the buffer in the address space of the process
  shared_circular_buffer = mmap(0, sizeof(struct circular_buf_t), PROT_READ | PROT_WRITE, MAP_SHARED, buffer_shm_fd, 0);

  // Open semaphores
  fill = sem_open(fill_sem_name, O_RDWR);
  avail = sem_open(avail_sem_name, O_RDWR);
  mutex = sem_open(mutex_sem_name, O_RDWR);

  //printf("Buffer semaphores have been initialized\n");
}

/**
* Read messages from the buffer
*/
void read_messages_from_buffer(double seconds_mean, char *data) {
  //printf("I'm gonna try to read a message\n");
  int message_index = 0;

  time_t beginSemaphore = time(NULL);

  sem_wait(fill);

  int sleepTime = ran_expo(seconds_mean);
  sleep(sleepTime);

  sem_wait(mutex);

  message_index = shared_circular_buffer->head;
  circular_buf_get(shared_circular_buffer, data);

  printf("***************************************************\n");
  printf("Consumer %i: A message has been read\n", getpid());
  printf("Consumer %i: Message from buffer: %s\n", getpid(), data);
  printf("Consumer %i: Index of the message in the buffer: %i\n", getpid(), message_index);
  printf("Consumer %i: there is/are %i consumer(s) alive\n", getpid(), * consumer_counter);
  read_producer_counter();
  printf("Consumer %i: there is/are %i producer(s) alive\n\n", getpid(), * producer_counter);

  sem_post(mutex);
  sem_post(avail);

  time_t endSemaphore = time(NULL);
  timeBlocked = timeBlocked + (endSemaphore - beginSemaphore) - sleepTime;
  total_time_sleeping = total_time_sleeping + sleepTime;

  counter_read_messages = counter_read_messages+1;
}

int main(int argc, char *argv[]) {
  begin = time(NULL);
  char data[256];
  char * endDecimalConvert;
  int opt;
  while ((opt = getopt(argc, argv, "m:n")) != -1)
  {
    switch (opt)
    {
    case 'n':
      buffer_name = strdup(argv[optind]);
      break;
    case 'm':
      meanConstant = strtod (optarg, & endDecimalConvert);
      if (endDecimalConvert == optarg) {
        printf ("-m parameter does not contain a valid number.\n");
        exit(0);
      }
      break;
    default:
      fprintf(stderr, "Usage: %s [-n buffer_name] [-m mean]\n", argv[0]);
      exit(EXIT_FAILURE);
    }
  }

  if (optind >= argc)
  {
    fprintf(stderr, "Expected argument after options\n\tUsage: %s [-n buffer_name] [-m mean]\n", argv[0]);
    exit(EXIT_FAILURE);
  }

  printf("Consumer %i has been initialized...\n", getpid());
  printf("Consumer %i: its buffer name is %s \n", getpid(), buffer_name);
  printf("Consumer %i: its mean value is %f\n\n", getpid(), meanConstant);

  // The first thing to do is to initialize the semaphores
  initialize_semaphores();
  initialize_buffer_semaphores();

  // The second thing to do is to increment the number of consumers alive
  read_and_modify_consumer_counter(true);

  // The third thing to do is to read messages from the buffer
  do {
    read_messages_from_buffer(meanConstant, data);
  } while(!is_killer_message(data));

  stop_consumer();
}
