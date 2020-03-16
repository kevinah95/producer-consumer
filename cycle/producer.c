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
  const char *name = "/k";
  int shm_fd; //shared memory file discriptor
  int buffer_size = 3;
  //size_t len = sizeof(struct circular_buf_t) * sizeof(char *) * buffer_size * sizeof(char) * 256;
  size_t len = sizeof(struct circular_buf_t) * sizeof(char) * buffer_size;

  /* make * shelf shared between processes*/
  //create the shared memory segment
  shm_fd = shm_open(name, O_RDWR, 0666);
  //configure the size of the shared memory segment
  ftruncate(shm_fd, len);
  //map the shared memory segment in process address space
  buffer_mem_ptr = mmap(0, len, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
  int fd = shm_open("/k_prime", O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
  ftruncate(fd, buffer_size*256);

  buffer_mem_ptr->buffer = mmap(0,buffer_mem_ptr->max*256,PROT_READ | PROT_WRITE,MAP_SHARED, fd,0);
  //buffer_mem_ptr->buffer = (char **)mmap(NULL, sizeof(char *) * buffer_size, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
  /* for(int i = 0; i < buffer_size; i++){
    buffer_mem_ptr->buffer[i] = (char *)mmap(NULL, sizeof(char) * buffer_size, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
  } */

  /* creat/open semaphores*/
  //cook post semaphore fill after cooking a pizza

  print_buffer_status(buffer_mem_ptr);
  printf("\nCook: I have started cooking pizza.\n");

  char data[256];
		    circular_buf_get(buffer_mem_ptr, data);
		    printf("%s ", data);


char *s = "datos";
    circular_buf_put(buffer_mem_ptr, s);
    print_buffer_status(buffer_mem_ptr);


    /* char *s = "datos";
    circular_buf_get(buffer_mem_ptr, s);
    print_buffer_status(buffer_mem_ptr); */



  return 0;
}
