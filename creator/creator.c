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
#include <inttypes.h>

static int producers_counter = 0;
static int consumers_counter = 0;
static int producer_flag = 0; //when a process is being produced.
static int consumer_flag = 0; //when a process is being consumed.
static int input_message_size = 0;
static circular_buf_t PRODUCERS_SHARED_MEMORY;
static circular_buf_t CONSUMERS_SHARED_MEMORY;
static sem_t con_counter_mutex;
static sem_t prod_counter_mutex;
uint8_t t;

int main(int argc, char *argv[]) {

    printf("Creador... \n");

    //porducer buffer size.
    printf("Type the producer buffer size: \n");
    scanf("%lu", &PRODUCERS_SHARED_MEMORY.max);

    //consumer buffer size.
    printf("Type the consumer buffer size: \n");
    scanf("%lu", &CONSUMERS_SHARED_MEMORY.max);

    //buffer name.
    printf("Type the producer buffer name \n");
    scanf("%s", &t);
    PRODUCERS_SHARED_MEMORY.buffer = t;

    //buffer name.
    printf("Type the consumer buffer name \n");
    scanf("%s", &t);
    CONSUMERS_SHARED_MEMORY.buffer = t;

    //producer buffer controller.
    circular_buf_init(PRODUCERS_SHARED_MEMORY.buffer, PRODUCERS_SHARED_MEMORY.max);

    //consumer buffer controller.
    circular_buf_init(CONSUMERS_SHARED_MEMORY.buffer, CONSUMERS_SHARED_MEMORY.max);

    //print producer buffer attributes
    printf("producer_buffer name: %s \n", &PRODUCERS_SHARED_MEMORY.buffer);
    printf("producer_buffer size: %lu \n", PRODUCERS_SHARED_MEMORY.max);

    printf("consumer_buffer name: %s \n", &CONSUMERS_SHARED_MEMORY.buffer);
    printf("consumer_buffer size: %lu \n", CONSUMERS_SHARED_MEMORY.max);

    printf("Number of produced processes: %d \n", producers_counter);
    printf("Number of consumed processes: %d \n", consumers_counter);

    //shared consumer and producer semaphores
    sem_init(&con_counter_mutex, 1, 1);
    sem_init(&prod_counter_mutex, 1, 1);

    sleep(1);
    exit(0);
}