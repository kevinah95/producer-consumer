CC = gcc

CFLAGS = -O3 -Wall -D_USE_PTHREADS -D_GNU_SOURCE -D_USE_PTHREADS_MEMINT -D_SHARED -D_USE_PTHREADS_MULTI #-D_UNROLL

LIBRARIES = -lm -lpthread

EXECNAME = killer

OBJS = killer.o

all: $(EXECNAME)

killer : $(OBJS)
	$(CC) $(CFLAGS) -o $(EXECNAME) $(OBJS) $(LIBRARIES)

killer.o: killer.c
	$(CC) $(CFLAGS) -c killer.c



clean:
	rm $(EXECNAME)
	rm $(OBJS)
