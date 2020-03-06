#include <stdio.h>
#include <stdlib.h>
#include <math.h> // math for exponential function
#include <time.h> // time for seeding
#include <pthread.h> // include pthread functions and structures
#include <semaphore.h> // include semaphores
#include <unistd.h> // include sleep
#include <stdint.h>
#include <stddef.h>
#include <assert.h>
#include "circular_buffer.h"
#include "circular_buffer.c"

static int producers_counter = 0;
static int consumers_counter = 0;
static int producer_flag = 0; //when a process is being produced.
static int consumer_flag = 0; //when a process is being consumed.
static int input_message_size = 0;
sem_t sem1;

int main(int argc, char *argv[]) {
    printf("Creador... \n");
    //shared semaphore
    sem_init(&sem1, 1, 1);

    sleep(1);
    exit(0);
}