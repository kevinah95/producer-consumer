# Producer - Consumer

The purpose of this project is to experiment with shared memory between heavyweight processes.

## Usage

### Creator
It's responsible for creating the shared circular buffer, and to initialize all the associated control variables (semaphores, flags, counters, etc.)

Use the next command in order to run the Creator:
```bash
./cycle/creator [-n buffer_name] [-s buffer_size]
```
The value of `buffer_size` must be an integer. A specific example looks like this:
```bash
./cycle/creator -n shm_shared_buffer -s 10
```

### Producer
It's a family of processes from the same implementation, they link to the buffer created by the Creator. They will produce messages that will be put in the circular buffer, this will be done with random sleeping time.

The messages will look like this: `21412-Date: 2020/3/15 Time: 13:12:43-2`

`21412`: it's the PID of the Producer

`Date: 2020/3/15 Time: 13:12:43`: it's the exact date and time when the message was produced.

`2`: it's a random key value from 0 to 4. 


Use the next command in order to run the Producer:
```bash
./cycle/producer [-n buffer_name] [-m mean_of_random_time]
```
The value of `buffer_name` must be the same used in the Creator. A specific example look like this:
```bash
./cycle/producer -n shm_shared_buffer -m 1
```

### Consumer
It's a family of processes from the same implementation, they link to the buffer created by the Creator. They will read messages from the circular buffer previously created, this will be done with random sleeping time.

Use the next command in order to run the Consumer:
```bash
./cycle/consumer [-n buffer_name] [-m mean_of_random_time]
```
The value of `buffer_name` must be the same used in the Creator. A specific example look like this:
```bash
./cycle/consumer -n shm_shared_buffer -m 1
```

### Killer ☠️
Program that cancels all the processes by sending special messages to the Consumer through the shared circular buffer, and by turning off a bit flag that indicates if the Producers should write messages.

Use the next command in order to run the Killer:
```bash
./cycle/killer [-n buffer_name]
```
The value of `buffer_name` must be the same used in the Creator. A specific example look like this:
```bash
./cycle/killer -n shm_shared_buffer
```

## Confessions
* The circular buffer was implement in a struct, it has a _well known_ size of 4096, however we simulate the action of setting the size of the buffer by the control provided by the semaphore, so for the user the will have the size they set as long as it's not bigger than 4096.

## Developed by
Kevin Hernández, [Steven Solano](https://github.com/solanors20), [Elisa Argueta](https://github.com/elisa7143), and [Jose Pablo Araya](https://github.com/arayajosepablo)
