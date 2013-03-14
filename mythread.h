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
#define THREAD_STACK_SIZE 6005
#define QUANTUM_N_SIZE 60000

unsigned long long getTime();
int mythread_switch();
void updateRuntimeOfThread(int thread_id);

enum thread_state {

  RUNNING,
  RUNNABLE,
  BLOCKED,
  EXIT
};

typedef struct _my_control_block {

	ucontext_t context;
	char * thread_name;
	int thread_id;
	enum thread_state state;
	unsigned long long start_time;
	unsigned long long run_time;

} my_control_block;

typedef struct _semaphore_t {

  List * thread_queue;
  int count;
  int initial;
} semaphore_t;



my_control_block thread_table[THREAD_MAX];
int current_thread_id;
int current_threads;

ucontext_t uctx_main;

List * runqueue;

int quantum_size;

semaphore_t semaphore_table[SEMAPHORE_MAX];
int current_semaphores;

/**
 * Returns the time elapsed since Jan 1, 1970 00:00:00 UTC
 * in nanoseconds.
 */
unsigned long long getTime() {

	struct timespec t;
	clock_gettime(CLOCK_REALTIME, &t);

	return t.tv_nsec;
}

void updateRuntimeOfThread(int thread_id) {

	unsigned long long t = getTime();
	unsigned long long delta_t = t - thread_table[thread_id].start_time;
	thread_table[thread_id].run_time += delta_t;
}

void scheduler() {

	// Sets the quantum timer
	struct itimerval t;
	t.it_interval.tv_sec = 0;
	t.it_interval.tv_usec = quantum_size;
	t.it_value.tv_sec = 0;
	t.it_value.tv_usec = quantum_size;
	setitimer(ITIMER_REAL, &t, 0);

	// Activates the thread switcher
	sigset(SIGALRM, &mythread_switch);
}

/**
 * Prints the state of all threads that are maintained by the library
 * at any given time.
 */
void mythread_state() {

	printf("Thread Info:\n");
	printf("%-16s %-12s %-16s\n","Name","State","Time");

	int id;
	char * state;
	for(id = 0; id < current_threads; id++) {
		//printf("%i\n", thread_table[id]->state);
		switch(thread_table[id].state) {

			case RUNNING: state = "RUNNING"; break;
			case RUNNABLE: state = "RUNNABLE"; break;
			case BLOCKED: state = "BLOCKED"; break;
			case EXIT: state = "EXIT"; break;
		
			default: state = "DEFAULT"; break;
		}

		//exit(0);
		printf("%s ", thread_table[id].thread_name);
		exit(0);
		printf("%-12s ", state);
		printf("%llu\n", thread_table[id].run_time);
		
	}

	printf("\n");
}

/**
 * Initialises all the global data structures for the thread system.
 */
int	mythread_init() {

	// and set the total number of active semaphore count to zero
	runqueue = list_create(NULL);
	current_semaphores = 0;
	current_threads = 0;

	return 0;
}

/**
 * Creates a new thread. Returns integer ID of the new thread control block.
 * Returns -1 and prints out an error message otherwise.
 */
int	mythread_create(char *threadname, void (*threadfunc) (), int stacksize) {

	int id = current_threads;

	// Checks input.
	if (stacksize > THREAD_STACK_SIZE) {
		printf("Error:Stack size is over limit.\n");

	} else if (current_threads > THREAD_MAX) {
		printf("Error:Max number of threads reached.\n");

	} else {
		// If no errors are found, proceed.

		// Sets up the user context appropriately.
		if(!getcontext(&thread_table[id].context)) {
			
			// Allocates the stack.
			thread_table[id].context.uc_link = &uctx_main;
			thread_table[id].context.uc_stack.ss_sp = malloc(stacksize);
			thread_table[id].context.uc_stack.ss_size = stacksize;
			thread_table[id].context.uc_stack.ss_flags = 0;
			sigemptyset(&thread_table[id].context.uc_sigmask);

			// Stores the thread properties in thread control block.
			thread_table[id].thread_name = threadname;
			thread_table[id].thread_id = id;
			thread_table[id].state = RUNNABLE;
			thread_table[id].start_time = getTime();
			thread_table[id].run_time = 0;

			//memcpy(&thread_table[id], &block, sizeof(block));

			// Runs the threadfunc() function when the thread starts.
			makecontext(&thread_table[id].context, threadfunc, 0);

			// the newly created thread is included in the runqueue.
			list_append_int(runqueue, id);

			printf("Thread #%d (%s) created successfully.\n", id, threadname);
			
			// Returns the id of the thread and update the number of threads.
			current_threads++;
			return id;

		} else {
			printf("Error:Cannot get context.\n");
		}
	}

	printf("Error:Creation of thread #%d was not successful.\n", id);
	return -1;
}

/**
 * Removes the thread from the runqueue. Called at the end
 * of the function that was invoked by the thread.
 * entry in the thread control block table could be still left
 * in there but state should be set to EXIT.
 */
void mythread_exit() {

	thread_table[current_thread_id].state = EXIT;
}

/**
 * Returns the ID of the current thread.
 */
int mythread_id() {

	return thread_table[current_thread_id].thread_id;
}

/**
 * Returns the ID of the current thread.
 */
int mythread_switch() {

	int old = current_thread_id;

	// In case all threads are done.
	if(list_empty(runqueue) && thread_table[old].state == EXIT) {
		
		updateRuntimeOfThread(old);
		printf("Success: All threads finished their job.\n");
		setcontext(&uctx_main);
	}

	// If there is some threads left.
    else if(!list_empty(runqueue)){

		int id = list_shift_int(runqueue);

		// Takes care of the running thread.
		if(thread_table[old].state == RUNNING) {
		
			thread_table[old].state = RUNNABLE;
		}
		updateRuntimeOfThread(old);
		list_append_int(runqueue, old);

		// Gets the next thread ready.
		if(thread_table[old].state == RUNNABLE) {
		
			thread_table[id].state = RUNNING;
		}
		thread_table[id].start_time = getTime();

		printf("Swapping from #%d to", old);
		printf(" to #%d.\n", id);

		current_thread_id = id;

		if(swapcontext(&thread_table[old].context, &thread_table[id].context)) {
			printf("Error:Swappint thread did not suceed.\n");
		}

		return id;
		
	}

	return -1;
}

/**
 * Switches control from the main thread
 * to one of the threads in the runqueue.
 * Also activates the thread switcher (interval timer that triggers
 * context switches every quantum nanoseconds).
 */
void runthreads() {

	// sigset_t sig;
	// sigemptyset(&sig);
	// sigaddset(&sig, SIGALRM);
	// sigprocmask(SIG_BLOCK, &sig, NULL); // Blocks signal

	scheduler();	

	// sigprocmask(SIG_UNBLOCK, &sig, NULL);

	if(!list_empty(runqueue)) {

		int id = list_shift_int(runqueue);

		thread_table[id].state = RUNNING;
		thread_table[id].start_time = getTime();

		printf("Starting to swap between threads.\n");

		if(swapcontext(&uctx_main, &thread_table[id].context) == -1) {

			printf("Error:Running thread #%d did not suceed.\n", id);
			return;
		}
	}

	// sigprocmask(SIG_BLOCK, &sig, NULL);
}

/**
 * Sets the quantum size of the round robin scheduler.
 */
void set_quantum_size(int quantum) {

	quantum_size = quantum;
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

	if (current_semaphores > SEMAPHORE_MAX) {
		return -1;
	}

	int id = current_semaphores;

	semaphore_t s = {
		.initial= value,
		.count = value,
		.thread_queue = list_create(NULL)};

	semaphore_table[id] = s;
	current_semaphores++;

	return id;
}

/**
 * Takes the calling thread out of the runqueue
 * if the value of the semaphore goes below 0.
 */
void semaphore_wait(int semaphore) {

	semaphore_t * s = &semaphore_table[semaphore];
	
	sigset_t sig;
	sigemptyset(&sig);
	sigaddset(&sig,SIGALRM);
	sigprocmask(SIG_BLOCK, &sig, NULL); // Blocks signal

	s->count--;
	if (s->count < 0) {

		thread_table[current_thread_id].state = BLOCKED;
		list_append(s->thread_queue, &thread_table[current_thread_id]);
		
		// Thread switch
	}

	sigprocmask(SIG_UNBLOCK, &sig, NULL);
}


/**
 * 
 */
void semaphore_signal(int semaphore) {

	semaphore_t * s = &semaphore_table[semaphore];
	
	sigset_t sig;
	sigemptyset(&sig);
	sigaddset(&sig, SIGALRM);
	sigprocmask(SIG_BLOCK, &sig, NULL); // Blocks signal

	s->count++;
	if (s->count <= 0) {

		thread_table[current_thread_id].state = RUNNABLE;
		list_append(runqueue, list_shift(s->thread_queue));
		
		mythread_switch();
	}

	sigprocmask(SIG_UNBLOCK, &sig, NULL);
}
/**
 * Removes a semaphore from the system.
 * A call to this function while threads are waiting
 * on the semaphore should fail.
 */
int destroy_semaphore(int semaphore) {

	semaphore_t * s = &semaphore_table[semaphore];

	if(list_empty(s->thread_queue)) {

		if(s->initial == s->count) {
			
			s->initial = 0;
			s->count = 0;
			list_release(s->thread_queue);

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