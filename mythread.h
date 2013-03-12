#ifndef __MY_THREADS_H__
#define __MY_THREADS_H__

#include <signal.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <sys/timeb.h>
#include <ucontext.h>
#include <slack/std.h>
#include <slack/list.h>


#define THREAD_NAME_LEN 100
#define THREAD_MAX 128
#define SEMAPHORE_MAX 128
#define THREAD_STACK_SIZE 4096
#define QUANTUM_N_SIZE 60000


/*
int	getcontext(ucontext_t *);
int	setcontext(const ucontext_t	* );	
void makecontext(ucontext_t	*, void	(*)(void), int, ...);	
int swapcontext(ucontext_t *, const ucontext_t *);
*/
enum thread_state {

  RUNNING,
  RUNNABLE,
  BLOCKED,
  EXIT
};

typedef struct _my_control_block {

	ucontext_t context;
	char thread_name[THREAD_NAME_LEN];
	int thread_id;
	enum thread_state state;
	struct timespec start_time;
	double run_time;

} my_control_block;

typedef struct _semaphore_t {

  List * thread_queue;
  int count;
  int initial;
} semaphore_t;



my_control_block ** thread_table;
int current_threads;
int my_threads_init;
ucontext_t uctx_main;
List * runqueue;
int quantum_size;
my_control_block * current_thread;
semaphore_t ** semaphore_table;
int current_semaphores;

/**
 * Initialises all the global data structures for the thread system.
 */
int	mythread_init() {

	//  initialises the semaphores table.
	semaphore_table = malloc(SEMAPHORE_MAX * sizeof semaphore_t);
	// and set the total number of active semaphore count to zero
	runqueue = list_create(NULL);
	current_semaphores = 0;

	return 0;
}

/**
 * Creates a new thread. Returns integer ID of the new thread control block.
 * Returns -1 and prints out an error message otherwise.
 */
int	mythread_create(char *threadname, void (*threadfunc) (), int stacksize) {

	/*
	This function is responsible for allocating the stack and setting up the user
	context appropriately. The newly created thread starts running the
	threadfunc() function when it starts.
	The threadname is stored in the thread control block and is printed
	for information purposes. A newly created thread is in the RUNNABLE state
	when it is inserted into the system. Depending on your system design, the
	newly created thread might be included in the runqueue.
	*/
	return 0;
}

/**
 * Removes the thread from the runqueue. Called at the end
 * of the function that was invoked by the thread.
 * entry in the thread control block table could be still left
 * in there but state should be set to EXIT.
 */
void mythread_exit() {

	;
}

/**
 * 
 */
int mythread_id() {

	return 0;
}
/**
 * Switches control from the main thread
 * to one of the threads in the runqueue.
 * Also activates the thread switcher ( interval timer that triggers
 * context switches every quantum nanoseconds.
 */
void runthreads() {

	/** Store context
	ualarm(quantum)?
	put current thread at ent of stack
	get next thread on stack
	swap context
	*/
	;
}

/**
 * Sets the quantum size of the round robin scheduler.
 */
void set_quantum_size(int quantum) {

	;
}

/**
 * Prints the state of all threads that are maintained by the library
 * at any given time.
 */
void mythread_state() {

	printf("Thread Info:\n");
	printf("Name:\t\t\n");
	printf("State:\t\t\n");
	printf("CPU Time:\t\n");
	;
}

void evict_thread() {
	;
}


/****************************************/
/*    SEMAPHORE STUFF                   */
/****************************************/

/**
 * Creates a semaphore and sets its initial value to the given value.
 * 
 */
int create_semaphore(int value) {


	return 0;
}

/**
 * Takes the calling thread  out of the runqueue
 * if the value of the semaphore goes below 0.
 */
void semaphore_wait(int semaphore) {

	semaphore_t * s = semaphore_table[semaphore];
	s->count--;

	if (s < 0) {
		list_append(s->thread_queue, current_thread);
		
		thread_switch();
	}
}


/**
 * 
 */
void semaphore_signal(int semaphore) {

	semaphore_t * s = semaphore_table[semaphore];
	s->count++;

	if (s->count <= 0) {
		list_append(runqueue, list_shift(s->thread_queue));
		
		//thread_switch();
	}
}
/**
 * Removes a semaphore from the system.
 * A call to this function while threads are waiting
 * on the semaphore should fail.

 */
int destroy_semaphore(int semaphore) {

	semaphore_t * s = semaphore_table[semaphore];

	if(list_empty(s->thread_queue)) {

		if(s->initial == s->count) {
			
			semaphore_t * fresh;
			s = fresh;

			return semaphore;
		} else {

			printf("Semaphore #%i was not empty on destruction.\n", semaphore);
		}
	} else {

		printf("Semaphore #%i has still some jobs waiting.\n", semaphore);
	}

	return 0;
}



#endif /* __MY_THREADS_H__ */