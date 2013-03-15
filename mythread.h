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
int current_thread_id, old, stop;
int current_threads;
ucontext_t uctx_main;
List * runqueue;
struct itimerval tvalue;
int quantum_size;
long long int switch_counter;
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

	// Activates the thread switcher
	sigset(SIGALRM, &mythread_switch);

	// Sets the quantum timer
	tvalue.it_interval.tv_sec = 0;
	tvalue.it_interval.tv_usec = quantum_size;
	tvalue.it_value.tv_sec = 0;
	tvalue.it_value.tv_usec = quantum_size;
	setitimer(ITIMER_REAL, &tvalue, 0);
}

/**
 * Prints the state of all threads that are maintained by the library
 * at any given time.
 */
void mythread_state() {

	printf("Thread Info:\n");
	printf("%-16s %-12s %-16s\n","Name","State","Time (ns)");

	int id;
	for(id = 0; id < current_threads; id++) {
	
		printf("%-16s ", thread_table[id].thread_name);

		switch(thread_table[id].state) {

			case RUNNING:
			printf("%-12s ", "RUNNING"); break;
			case RUNNABLE:
			printf("%-12s ", "RUNNABLE"); break;
			case BLOCKED:
			printf("%-12s ", "BLOCKED"); break;
			case EXIT:
			printf("%-12s ", "EXIT"); break;
			default:
			printf("%-12s ", "DEFAULT"); break;
		}
		;
		printf("%-16llu\n", thread_table[id].run_time);
		
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
	current_thread_id = 0;
	switch_counter = 0;
	quantum_size = 0;
	stop = 0;
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
			thread_table[id].start_time = 0;
			thread_table[id].run_time = 0;

			// Runs the threadfunc() function when the thread starts.
			makecontext(&thread_table[id].context, threadfunc, 0);

			// the newly created thread is included in the runqueue.
			list_append_int(runqueue, id);

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

	// Updates the switch_counter.
	switch_counter++;

	// In case all threads are done.
	if(list_empty(runqueue) && thread_table[current_thread_id].state == EXIT) {
		
		updateRuntimeOfThread(current_thread_id);
		stop++;
		setcontext(&uctx_main);
	}

	// If there is some threads left.
    else if(!list_empty(runqueue)){
		
    	int old = current_thread_id;
	
		// Takes care of the running thread.
		updateRuntimeOfThread(current_thread_id);

		if(thread_table[current_thread_id].state == RUNNABLE
			|| thread_table[current_thread_id].state == RUNNING){

			list_append_int(runqueue, current_thread_id);
		}

		// Gets the next thread ready.
		current_thread_id = list_shift_int(runqueue);
		
		thread_table[current_thread_id].start_time = getTime();
		
		if(swapcontext(&thread_table[old].context,
			&thread_table[current_thread_id].context)) {

			printf("Error:Swappint thread did not suceed.\n");
		}

		return current_thread_id;
	}

	return -3;
}

/**
 * Switches control from the main thread
 * to one of the threads in the runqueue.
 * Also activates the thread switcher (interval timer that triggers
 * context switches every quantum nanoseconds).
 */
void runthreads() {


	scheduler();	

	current_thread_id = list_shift_int(runqueue);

	thread_table[current_thread_id].start_time = getTime();

	if(swapcontext(&uctx_main,
		&thread_table[current_thread_id].context) == -1) {

		printf("Error:Running thread #%d did not suceed.\n", current_thread_id);
		return;
	}

	while(!stop);
}

/**
 * Sets the quantum size of the round robin scheduler.
 */
void set_quantum_size(int quantum) {
	quantum_size = quantum;
}


/********************************************************************/
/*                    SEMAPHORE STUFF                               */
/********************************************************************/
/**
 * Creates a semaphore and sets its initial value to the given value.
 */
int create_semaphore(int value) {

	if (current_semaphores > SEMAPHORE_MAX) {
		return -1;
	}
	semaphore_table[current_semaphores] = *(semaphore_t *) malloc( sizeof(semaphore_t));
	semaphore_table[current_semaphores].initial = value;
	semaphore_table[current_semaphores].count = value;
	semaphore_table[current_semaphores].thread_queue = list_create(NULL);

	return current_semaphores++;
}

/**
 * Takes the calling thread out of the runqueue
 * if the value of the semaphore goes below 0.
 */
void semaphore_wait(int s) {

	sigset_t sig;
	sigemptyset(&sig);
	sigaddset(&sig,SIGALRM);
	sigprocmask(SIG_BLOCK, &sig, NULL); // Blocks signal

	//Record the current switch_counter
    long long int old_counter = switch_counter;

	semaphore_table[s].count--;
	if (semaphore_table[s].count < 0) {

		thread_table[current_thread_id].state = BLOCKED;
		list_append_int(semaphore_table[s].thread_queue, current_thread_id);
	}
	sigprocmask(SIG_UNBLOCK, &sig, NULL);

	// Waits until the scheduler deals with it.
    while(old_counter == switch_counter);
}


/**
 * 
 */
void semaphore_signal(int s) {

	semaphore_table[s].count++;

	if (semaphore_table[s].count <= 0) {

		int thread_id = list_shift_int(semaphore_table[s].thread_queue);
		thread_table[thread_id].state = RUNNABLE;
		list_append_int(runqueue, thread_id);
	}
}
/**
 * Removes a semaphore from the system.
 * A call to this function while threads are waiting
 * on the semaphore should fail.
 */
int destroy_semaphore(int s) {

	if(&semaphore_table[s] != NULL) {

		if(list_empty(semaphore_table[s].thread_queue)) {

			if(semaphore_table[s].initial != semaphore_table[s].count) {
				printf("Semaphore #%i was not empty on destruction.\n", s);
			}
			
			semaphore_table[s].initial = 0;
			semaphore_table[s].count = 0;
			list_release(semaphore_table[s].thread_queue);
			//free(&semaphore_table[s]);

			return s;
			
		} else {

			printf("Semaphore #%i has still some jobs waiting.\n", s);
		}
	}
	return 0;
}

#endif /* __MY_THREADS_H__ */