PROG=mythread
CC=gcc
CC_FLAGS=-g -O2 -c -Wall -DHAVE_PTHREAD_RWLOCK=1 -lslack -lm
OBJECTS=$(PROG).o
DEPS = mythread.h

all:$(OBJECTS)
	$(CC) $(PROG).o -o $(PROG)

$(OBJECTS):%.o:%.c
	$(CC) $(CC_FLAGS) $< -o $@

tester: mythread.o
	gcc mythread.c mythread.h -DHAVE_PTHREAD_RWLOCK=1 -lslack -lm -o mythread

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CC_FLAGS)

clean:
	rm *.o
	rm -rf $(PROG)

