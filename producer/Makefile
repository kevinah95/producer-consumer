CC = gcc

CFLAGS = -O3 -Wall -D_USE_PTHREADS -D_GNU_SOURCE -D_USE_PTHREADS_MEMINT -D_SHARED -D_USE_PTHREADS_MULTI #-D_UNROLL

LIBRARIES = -lm -lpthread

EXECNAME = producer

OBJS = producer.o

all: $(EXECNAME)

producer : $(OBJS)
	$(CC) $(CFLAGS) -o $(EXECNAME) $(OBJS) $(LIBRARIES)

producer.o: producer.c
	$(CC) $(CFLAGS) -c producer.c



clean:
	rm $(EXECNAME)
	rm $(OBJS)