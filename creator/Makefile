CC = gcc

CFLAGS = -O3 -Wall -D_USE_PTHREADS -D_GNU_SOURCE -D_USE_PTHREADS_MEMINT -D_SHARED -D_USE_PTHREADS_MULTI #-D_UNROLL

LIBRARIES = -lm -lpthread

EXECNAME = creator

OBJS = creator.o

all: $(EXECNAME)

creator : $(OBJS)
	$(CC) $(CFLAGS) -o $(EXECNAME) $(OBJS) $(LIBRARIES)

creator.o: creator.c
	$(CC) $(CFLAGS) -c creator.c



clean:
	rm $(EXECNAME)
	rm $(OBJS)
