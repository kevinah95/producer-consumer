CC      := gcc
CCFLAGS := 
LDFLAGS := -lpthread -lm -lrt

TARGETS:= creator cook waiter
MAINS  := $(addsuffix .o, $(TARGETS) )
OBJ    := creator.o $(MAINS)
DEPS   := 

.PHONY: all clean

all: $(TARGETS)

clean:
	rm -f $(TARGETS) $(OBJ)

$(OBJ): %.o : %.c $(DEPS)
	$(CC) -c -o $@ $< $(CCFLAGS)

$(TARGETS): % : $(filter-out $(MAINS), $(OBJ)) %.o
	$(CC) -o $@ $(LIBS) $^ $(CCFLAGS) $(LDFLAGS)