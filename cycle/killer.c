#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h> // math for exponential function
#include <time.h> // time for seeding
#include <pthread.h> // include pthread functions and structures
#include <semaphore.h> // include semaphores
#include <unistd.h> // include sleep
#include "../shared/shared.h"
#include "../circular_buffer/circular_buffer.h"
#include "../circular_buffer/circular_buffer.c"

int main(int argc, char *argv[]) {
  printf("Killer...");

  sleep(1);
  exit(0);
}
