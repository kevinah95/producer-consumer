#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <math.h> // Math for exponential function
#include <time.h> // Time for seeding
#include <pthread.h> // Pthread functions and structures
#include <semaphore.h> // Semaphores
#include <unistd.h> // Sleep
#include <sys/mman.h> // Shared memories functions
#include <semaphore.h>
#include <fcntl.h>
#include <sys/shm.h>
#include <sys/stat.h>

#define CONSUMERS_SHARED_STORAGE_ID "/CONSUMERS_SHARED_MEMORY"
#define PRODUCERS_SHARED_STORAGE_ID "/p_mem"

#define EXAMPLE_BUFFER_SIZE 10

const char special_message[] = "KILLER_MESSAGE_FROM_FINISHER"; // Special message that indicates the process must finisha

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

// >>Begin: Buffer struct and methods
struct circular_buf_t
{
  char buffer[EXAMPLE_BUFFER_SIZE][256];
  size_t head;
  size_t tail;
  size_t max; // of the buffer
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

void retreat_pointer(struct circular_buf_t * cbuf)
{
	cbuf->full = false;
	cbuf->tail = (cbuf->tail + 1) % cbuf->max;
}

int circular_buf_get(struct circular_buf_t * cbuf, char * data)
{
    int r = -1;

    if(!circular_buf_empty(cbuf))
    {
      strcpy(data,cbuf->buffer[cbuf->tail]);
      retreat_pointer(cbuf);

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
// << End: Buffer struct and methods

struct circular_buf_t * shared_circular_buffer;
sem_t * fill, * avail, * mutex;

/**
* Initializes mutex semaphores to control the counters of shared memory
*/
void initialize_semaphores() {
  sem_con_producer_mutex = sem_open(prod_counter_mutex, O_CREAT, 0644, 1);
  sem_con_counter_mutex = sem_open(con_counter_mutex, O_CREAT, 0644, 1);
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
  shm_unlink(CONSUMERS_SHARED_STORAGE_ID);
  shm_unlink(PRODUCERS_SHARED_STORAGE_ID);
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
  c_shm_fd = shm_open(CONSUMERS_SHARED_STORAGE_ID, O_RDWR, S_IRUSR | S_IWUSR);
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

  printf("PID %d: Read from shared memory: \"%d\"\n", pid, *consumer_counter);

  sem_wait(sem_con_counter_mutex);
  if(isIncrement) {
    (* consumer_counter)++;
  } else {
    (* consumer_counter)--;
  }
  sem_post(sem_con_counter_mutex);

  return 0;
}

/**
* Read from shared memory the counter of producers
*/
int read_producer_counter() {
  pid_t pid;
  pid = getpid();

  // Get shared memory file descriptor (not a file)
  p_shm_fd = shm_open(PRODUCERS_SHARED_STORAGE_ID, O_RDWR, S_IRUSR | S_IWUSR);
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
  printf("Consumer PID %i: this consumer has been stopped\n", getpid());
  printf("Consumer PID %i: this consumer remained alive %ld second(s)\n", getpid(), (time(NULL) - begin));
  printf("Consumer PID %i: this consumer read %d message(s)\n", getpid(), counter_read_messages);
  printf("Consumer PID %i: there is/are %i consumer(s) alive\n", getpid(), * consumer_counter);
  read_producer_counter();
  printf("Consumer PID %i: there is/are %i producer(s) alive\n", getpid(), * producer_counter);
  //close_semaphores();
  //close_shared_memory();
  sleep(1);
  exit(0);
}

void initialize_buffer_semaphores() {
  const char * name = "shared_memory";
  const char * sema1= "fill";
  const char * sema2= "avail";
  const char * sema3= "mutex";

  int buffer_shm_fd; // Buffer (shared memory) file discriptor

  // Open the shared memory segment
  buffer_shm_fd = shm_open(name, O_RDWR, 0666);
  ftruncate(buffer_shm_fd, sizeof(struct circular_buf_t));

  // Now map the shared memory segment of the buffer in the address space of the process
  shared_circular_buffer = mmap(0, sizeof(struct circular_buf_t), PROT_READ | PROT_WRITE, MAP_SHARED, buffer_shm_fd, 0);

  // Open semaphores
  fill = sem_open(sema1, O_CREAT,0666,0);
  avail = sem_open(sema2, O_CREAT, 0666, 3);
  mutex = sem_open(sema3,O_CREAT,0666,1);
}

/**
* Read messages from the buffer
*/
void read_messages_from_buffer(double seconds_mean, char *data) {
  sem_wait(fill);
  sleep(ran_expo(seconds_mean));
  sem_wait(mutex);
  circular_buf_get(shared_circular_buffer, data);
  printf("Message from buffer: %s\n", data);
  print_buffer_status(shared_circular_buffer);
  sem_post(mutex);
  sem_post(avail);
  counter_read_messages = counter_read_messages+1;
}

int main(int argc, char *argv[]) {
  begin = time(NULL);
  char data[256];
  printf("Consumer...\n");

  // The first thing to do is to initialize the semaphores
  initialize_semaphores();
  initialize_buffer_semaphores();

  // The second thing to do is to increment the number of consumers alive
  read_and_modify_consumer_counter(true);

  // The third thing to do is to read messages from the buffer
  do {
    read_messages_from_buffer(1.0, data);
  } while(!is_killer_message(data));

  stop_consumer();
}
