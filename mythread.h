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
	unsigned long long delta_t = t - thread_table[thread_id]->start_time;
	thread_table[thread_id]->run_time += delta_t;
}
/**
 * Initialises all the global data structures for the thread system.
 */
int	mythread_init() {

	//  initialises the semaphores table.
	semaphore_table = malloc(SEMAPHORE_MAX * sizeof(semaphore_t));

	// and set the total number of active semaphore count to zero
	runqueue = list_create(NULL);
	current_semaphores = 0;
	current_threads = 0;

	thread_table = malloc(THREAD_MAX * sizeof(my_control_block));

	return 0;
}

/**
 * Creates a new thread. Returns integer ID of the new thread control block.
 * Returns -1 and prints out an error message otherwise.
 */
int	mythread_create(char *threadname, void (*threadfunc) (), int stacksize) {

	int id = current_threads;
	my_control_block block;
	ucontext_t context;

	// Checks input.
	if (stacksize > THREAD_STACK_SIZE) {
		printf("Error:Max thread stack reached.\n");

	} else if (current_threads > THREAD_MAX) {
		printf("Error:Max number of threads reached.\n");

	} else {
		// If no errors are found, proceed.

		// Sets up the user context appropriately.
		if(!getcontext(&context)) {
			
			// Allocates the stack.
			context.uc_link = &uctx_main;
			context.uc_stack.ss_sp = malloc(stacksize);
			context.uc_stack.ss_size = stacksize;
			context.uc_stack.ss_flags = 0;
			sigemptyset(&context.uc_sigmask);

			// Link to the control block.
			block.context = context;

			// Stores the thread properties in thread control block.
			block.thread_name = threadname;
			block.thread_id = id;
			block.state = RUNNABLE;
			block.start_time = getTime();
			block.run_time = 0;

			// Runs the threadfunc() function when the thread starts.
			makecontext(&block.context, threadfunc, 0);

			thread_table[id] = &block;

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

	current_thread->state = EXIT;
}

/**
 * Returns the ID of the current thread.
 */
int mythread_id() {

	return current_thread->thread_id;
}
/**
 * Switches control from the main thread
 * to one of the threads in the runqueue.
 * Also activates the thread switcher ( interval timer that triggers
 * context switches every quantum nanoseconds.
 */
void runthreads() {

	/** Store contextw
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

	quantum_size = quantum;
}

/**
 * Prints the state of all threads that are maintained by the library
 * at any given time.
 */
void mythread_state() {

	printf("Thread Info:\n");
	printf("%-16s %-12s %-16s","Name","State","Time");

	int id;
	char * state;
	for(id = 0; id < current_threads; id++) {
		
		switch(thread_table[id]->state) {

			case RUNNING: state = "RUNNING"; break;
			case RUNNABLE: state = "RUNNABLE"; break;
			case BLOCKED: state = "BLOCKED"; break;
			case EXIT: state = "EXIT"; break;
		
			default: state = "DEFAULT"; break;
		}
		printf("%-16s ", thread_table[id]->thread_name);
		printf("%-12s ", state);
		printf("%llu\n", thread_table[id]->run_time);
	}

	printf("\n");
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

	semaphore_table[id]->initial = value;
	semaphore_table[id]->count = value;
	semaphore_table[id]->thread_queue = list_create(NULL);
	
	current_semaphores++;
	return id;
}

/**
 * Takes the calling thread out of the runqueue
 * if the value of the semaphore goes below 0.
 */
void semaphore_wait(int semaphore) {

	semaphore_t * s = semaphore_table[semaphore];
	
	sigset_t sig;
	sigemptyset(&sig);
	sigaddset(&sig,SIGALRM);
	sigprocmask(SIG_BLOCK,&sig,NULL); // Blocks signal

	s->count--;
	if (s->count < 0) {

		current_thread->state = BLOCKED;
		list_append(s->thread_queue, current_thread);
		
		// Thread switch
	}

	sigprocmask(SIG_UNBLOCK,&sig,NULL);
}


/**
 * 
 */
void semaphore_signal(int semaphore) {

	semaphore_t * s = semaphore_table[semaphore];
	
	sigset_t sig;
	sigemptyset(&sig);
	sigaddset(&sig,SIGALRM);
	sigprocmask(SIG_BLOCK,&sig,NULL); // Blocks signal

	s->count++;
	if (s->count <= 0) {

		current_thread->state = RUNNABLE;
		list_append(runqueue, list_shift(s->thread_queue));
		
		//thread_switch();
	}

	sigprocmask(SIG_UNBLOCK,&sig,NULL);
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