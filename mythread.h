#define THREAD_NAME_LEN 100


/*
int	getcontext(ucontext_t *);
int	setcontext(const ucontext_t	* );	
void makecontext(ucontext_t	*, void	(*)(void), int, ...);	
int swapcontext(ucontext_t *, const ucontext_t *);
*/

typedef struct _mythread_control_block {

	ucontext_t context;
	char thread_name[THREAD_NAME_LEN];
	int thread_id;
	char state;
	int cpu_time;

} mythread_control_block;

/**
 * Initializes all the global data structures for the thread system.
 */
int	mythread_init() {

	;
}

/**
 * Returns an integer pointing to the thread control block allocated to
 * the newly created thread in the thread control block table. (-1 on err)
 */
int	mythread_create(char *threadname, void (*threadfunc) (), int stacksize) {

	;
}

/**
 * 
 */
void mythread_exit() {

	;
}

/**
 * 
 */
void runthreads() {

	;
}

/**
 * 
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


/****************************************/
/*    SEMAPHORE STUFF                   */
/****************************************/

typedef struct semaphore_t {

	List queue;
	int count;

};

/**
 * 
 */
int create_semaphore(int value) {

	;
}

/**
 * 
 */
int destroy_semaphore(int value) {

	;
}

/**
 * 
 */
void semaphore_wait(semaphore_t * s) {

	s->count--;

	if (s->count < 0) {
		enqueue(s->count, current_thread);
		
		thread_switch();
	}
}


/**
 * 
 */
void semaphore_signal(semaphore_t * s) {

	s->count++;

	if (s->count <= 0) {
		enqueue(runqueue, dequeue(s->queue));
		
		thread_switch();
	}
}
