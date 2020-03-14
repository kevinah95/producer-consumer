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
  const char *name = "k";
  int shm_fd; //shared memory file discriptor

  cbuf_handle_t shared_mem_ptr;

  /* make * shelf shared between processes*/
  //create the shared memory segment
  shm_fd = shm_open(name, O_RDWR, 0666);
  //configure the size of the shared memory segment
  ftruncate(shm_fd, sizeof(cbuf_handle_t));
  //map the shared memory segment in process address space
  shared_mem_ptr = mmap(0, sizeof(cbuf_handle_t), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);

  /* creat/open semaphores*/
  //cook post semaphore fill after cooking a pizza

  print_buffer_status(shared_mem_ptr);
  printf("\nCook: I have started cooking pizza.\n");



    /* char data[256];
		    circular_buf_get(shared_mem_ptr, data);
		    printf("%s ", data); */
    char *s = "datos";
    circular_buf_put(shared_mem_ptr, s);
    print_buffer_status(shared_mem_ptr);



  return 0;
}
