CC = gcc

CFLAGS = -O3 -Wall -D_USE_PTHREADS -D_GNU_SOURCE -D_USE_PTHREADS_MEMINT -D_SHARED -D_USE_PTHREADS_MULTI #-D_UNROLL

LIBRARIES = -lm -lpthread

EXECNAME = consumer

OBJS = consumer.o

all: $(EXECNAME)

consumer : $(OBJS)
	$(CC) $(CFLAGS) -o $(EXECNAME) $(OBJS) $(LIBRARIES)

consumer.o: consumer.c
	$(CC) $(CFLAGS) -c consumer.c



clean:
	rm $(EXECNAME)
	rm $(OBJS)
