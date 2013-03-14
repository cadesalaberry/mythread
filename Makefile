CFLAGS := -std=gnu99 -g \
	-Wall -Wextra -lslack \
	-DHAVE_PTHREAD_RWLOCK=1 \
	${CFLAGS}

LDLIBS = -lm -lslack -lrt

all: mythread
mythreads: mythread.o

clean:
	${RM} mythread *.o

.PHONY: clean install uninstall

