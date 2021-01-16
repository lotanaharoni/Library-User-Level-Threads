
#include "Thread.h"
#include "uthreads.h"
#include <iostream>
#include <vector>
#include <list>
#include <sys/time.h>
#include <signal.h>


#define ERROR_1 "system error: system call unsuccessful\n"
#define ERROR_2 "thread library error: invalid input\n"
#define ERROR_3 "thread library error: max thread num reached\n"

#define MICRO_SEC 1000000



static std::vector<Thread*> thread_pool;
static std::list<Thread*> ready_queue;
Thread* running_thread;
int counter;
int * quantums;
int quantums_size;
struct sigaction sa = {};
struct itimerval timer;
sigset_t set;


/**
 * This function removes a thread from the ready queue.
 * @param tid - the thread id.
 */
void remove_thread_from_queue(int tid){
	std::list<Thread*>::iterator erase;
	for(auto it=ready_queue.begin(); it!=ready_queue.end(); ++it)
	{
		if ((*it)->get_id() == tid)
		{
			erase = it;
		}
	}
	ready_queue.erase(erase);
}

/**
 * This function exits the program.
 * @param _exit the of exit code
 */
void exit_library (int _exit)
{
	for(auto & i : thread_pool)
	{
		delete i;
	}
	delete [] quantums;
	if (_exit)
	{
		fprintf(stderr, ERROR_1);
	}
	exit(_exit);

}


/**
 * This function creates a new thread.
 * @param f - the function this new thread is to operate
 * @param priority - the priority of the new thread.
 * @return - the id of the created thread.
 */
int create_thread(void (*f)(), int priority){

	for(unsigned long i=0; i < thread_pool.size(); i++)
	{

		/* Searching for a terminated thread to replace*/
		if (thread_pool[i]->get_terminated())
		{
			Thread* temp = thread_pool[i];

			/*replacing the terminated thread with a new one*/
			try{
				thread_pool[i] = new Thread ((int)i, priority, f);
			}
			catch (std::bad_alloc&){
				sigprocmask(SIG_UNBLOCK, &set, nullptr);
				exit_library(1);
			}
			delete temp;

			/*returning id*/
			return (int)i;
		}
	}

	/* Checking if the thread pool is full*/
	if (thread_pool.size() == MAX_THREAD_NUM)
	{
		return -1;
	}

	/*if not - allocating a new thread*/
	try{
		thread_pool.push_back(new Thread (thread_pool.size(), priority, f));
	}
	catch (std::bad_alloc&){
		sigprocmask(SIG_UNBLOCK, &set, nullptr);
		exit_library(1);
	}

	/*returning the new id*/
	return (int)thread_pool.size()-1;
}


/**
 * This function is the signal handler. it switches between
 * the running thread and the thread on the head of the ready queue.
 * this function is called only by the signal
 * @param sig - the signal number
 */
void replace_running_thread(int sig){
	/*blocking signals*/
	if(sigprocmask(SIG_BLOCK, &set, nullptr) == -1){
		exit_library(1);
	}

	/*if there are no other threads beside id-0, then no switch is necessary*/
	if (ready_queue.empty()){
		timer.it_value.tv_sec = quantums[running_thread->get_priority()] / MICRO_SEC;
		timer.it_value.tv_usec = quantums[running_thread->get_priority()] % MICRO_SEC;
		running_thread->set_state(RUNNING);
		counter++;

		/*setting a new timer*/
		if (setitimer (ITIMER_VIRTUAL, &timer, nullptr)) {
			sigprocmask(SIG_UNBLOCK, &set, nullptr);
			exit_library(1);
		}
		return;
	}

	/*switching thread objects*/
	Thread* stopped = running_thread;
	running_thread = ready_queue.front();
	running_thread->set_state(RUNNING);
	counter++;
	ready_queue.pop_front();
	if(!stopped->get_terminated())
	{
		if(stopped->get_state()!=BLOCKED)
		{
			stopped->set_state(READY);
			ready_queue.push_back(stopped);
		}
	}

	/*saving running thread*/
	int ret_val = sigsetjmp(stopped->get_env(),1);
	if (ret_val == 1) {
		return;
	}

	/*setting a new timer*/
	timer.it_value.tv_sec = quantums[running_thread->get_priority()] / MICRO_SEC;
	timer.it_value.tv_usec = quantums[running_thread->get_priority()] % MICRO_SEC;
	if (setitimer (ITIMER_VIRTUAL, &timer, nullptr)) {
		sigprocmask(SIG_UNBLOCK, &set, nullptr);
		exit_library(1);
	}

	/*loading a new thread*/
	siglongjmp(running_thread->get_env(),1);
}



int uthread_init(int *quantum_usecs, int size){

	/*input check*/
	if (size <= 0 || quantum_usecs == nullptr){
		fprintf(stderr, ERROR_2);
		return -1;
	}
	for (int i=0; i < size; ++i){
		if (*(quantum_usecs+i) <= 0){
			fprintf(stderr, ERROR_2);
			return -1;
		}
	}

	/*Deep copy for array*/
	quantums_size = size;
	quantums = new int [quantums_size];
	for (int i=0; i < size; i++)
	{
		quantums[i]=quantum_usecs[i];
	}

	/*initialize the sig_block set*/
	if (sigemptyset(&set) == -1)
	{
		exit_library(1);
	}
	if (sigaddset(&set, SIGVTALRM) == -1)
	{
		exit_library(1);
	}

	/*attach the handler to the signal*/
	sa.sa_handler = &replace_running_thread;
	if (sigaction(SIGVTALRM, &sa,nullptr) < 0) {
		exit_library(1);
	}

	/*creating first thread with id-0*/
	try{
		thread_pool.push_back(new Thread (0, 0, nullptr));
	}
	catch (std::bad_alloc&){
		sigprocmask(SIG_UNBLOCK, &set, nullptr);
		exit_library(1);
	}
	running_thread = thread_pool[0];
	thread_pool[0]->set_state(RUNNING);
	counter = 1;
    timer.it_value.tv_sec = quantums[running_thread->get_priority()] / MICRO_SEC;
    timer.it_value.tv_usec = quantums[running_thread->get_priority()] % MICRO_SEC;
    if (setitimer (ITIMER_VIRTUAL, &timer, nullptr)) {
		sigprocmask(SIG_UNBLOCK, &set, nullptr);
		exit_library(1);
    }

	/*running thread*/
	return 0;
}


int uthread_spawn(void (*f)(), int priority){

	/*blocking signals*/
	if(sigprocmask(SIG_BLOCK, &set, nullptr) == -1){
		exit_library(1);
	}

	/*input check*/
	if (priority >= quantums_size || priority < 0)
	{
		if(sigprocmask(SIG_UNBLOCK, &set, nullptr) == -1){
			exit_library(1);
		}
		fprintf(stderr, ERROR_2);
		return -1;
	}

	/*creating the new thread*/
	int id = create_thread(f, priority);

	/*adding the new thread to ready_queue*/
	if (id != -1)
	{
		ready_queue.push_back(thread_pool[id]);
		if(sigprocmask(SIG_UNBLOCK, &set, nullptr) == -1){
			exit_library(1);
		}
		return id;
	}

	/*in case of unsuccessful creation*/
	else{
		if(sigprocmask(SIG_UNBLOCK, &set, nullptr) == -1){
			exit_library(1);
		}
		fprintf(stderr, ERROR_3);
		return -1;
	}
}


int uthread_change_priority(int tid, int priority){

	/*blocking signals*/
	if(sigprocmask(SIG_BLOCK, &set, nullptr) == -1){
		exit_library(1);
	}

	/*input check*/
	if(tid < 0 || priority >= quantums_size || priority < 0 || tid >= (int)thread_pool.size())
	{
		if(sigprocmask(SIG_UNBLOCK, &set, nullptr) == -1){
			exit_library(1);
		}
		fprintf(stderr, ERROR_2);
		return -1;
	}

	/*finding the required thread*/

	if (!(thread_pool[tid]->get_terminated()))
	{
		thread_pool[tid]->set_priority(priority);
		if(sigprocmask(SIG_UNBLOCK, &set, nullptr) == -1){
			exit_library(1);
		}
		return 0;
	}

	/*if required id not found*/
	if(sigprocmask(SIG_UNBLOCK, &set, nullptr) == -1){
		exit_library(1);
	}
	fprintf(stderr, ERROR_2);
	return -1;
}


int uthread_terminate(int tid){

	/*blocking signals*/
	if(sigprocmask(SIG_BLOCK, &set, nullptr) == -1){
		exit_library(1);
	}

	/*input check*/
	if (tid == 0)
	{
		if(sigprocmask(SIG_UNBLOCK, &set, nullptr) == -1){
			exit_library(1);
		}
		exit_library(0);
	}
	if(tid < 0 || tid >= (int)thread_pool.size())
	{
		if(sigprocmask(SIG_UNBLOCK, &set, nullptr) == -1){
			exit_library(1);
		}
		fprintf(stderr, ERROR_2);
		return -1;
	}

	/*finding the required thread*/
	if (!(thread_pool[tid]->get_terminated()))
	{
		thread_pool[tid]->set_terminated();
		if (thread_pool[tid]->get_state() == RUNNING)
		{
			if(sigprocmask(SIG_UNBLOCK, &set, nullptr) == -1){
				exit_library(1);
			}
			raise(SIGVTALRM);
		}
		else
		{
			if (thread_pool[tid]->get_state() == READY)
			{
				remove_thread_from_queue(tid);
			}
		}
		if(sigprocmask(SIG_UNBLOCK, &set, nullptr) == -1){
			exit_library(1);
		}
		return 0;
	}

	/*if required id not found*/
	if(sigprocmask(SIG_UNBLOCK, &set, nullptr) == -1){
		exit_library(1);
	}
	fprintf(stderr, ERROR_2);
	return -1;
}


int uthread_block(int tid){

	/*blocking signals*/
	if(sigprocmask(SIG_BLOCK, &set, nullptr) == -1){
		exit_library(1);
	}

	/*input check*/
	if (tid <= 0 || tid >= (int)thread_pool.size())
	{
		if(sigprocmask(SIG_UNBLOCK, &set, nullptr) == -1){
			exit_library(1);
		}
		fprintf(stderr, ERROR_2);
		return -1;
	}

	/*finding the required thread*/
	if (!(thread_pool[tid]->get_terminated()))
	{
		if (thread_pool[tid]->get_state() == RUNNING)
		{
			thread_pool[tid]->set_state(BLOCKED);
			if(sigprocmask(SIG_UNBLOCK, &set, nullptr) == -1){
				exit_library(1);
			}
			raise(SIGVTALRM);
		}
		else
		{
			if (thread_pool[tid]->get_state() == READY)
			{
				remove_thread_from_queue(tid);
			}
			thread_pool[tid]->set_state(BLOCKED);
		}
		if(sigprocmask(SIG_UNBLOCK, &set, nullptr) == -1){
			exit_library(1);
		}
		return 0;
	}

	/*if required id not found*/
	if(sigprocmask(SIG_UNBLOCK, &set, nullptr) == -1){
		exit_library(1);
	}
	fprintf(stderr, ERROR_2);
	return -1;
}


int uthread_resume(int tid){

	/*blocking signals*/
	if(sigprocmask(SIG_BLOCK, &set, nullptr) == -1){
		exit_library(1);
	}

	/*input check*/
	if(tid < 0 || tid >= (int)thread_pool.size())
	{
		if(sigprocmask(SIG_UNBLOCK, &set, nullptr) == -1){
			exit_library(1);
		}
		fprintf(stderr, ERROR_2);
		return -1;
	}

	/*finding the required thread*/
	if (!(thread_pool[tid]->get_terminated()))
	{
		if (thread_pool[tid]->get_state() == BLOCKED)
		{
			thread_pool[tid]->set_state(READY);
			ready_queue.push_back(thread_pool[tid]);
		}
		if(sigprocmask(SIG_UNBLOCK, &set, nullptr) == -1){
			exit_library(1);
		}
		return 0;
	}

	/*if required id not found*/
	if(sigprocmask(SIG_UNBLOCK, &set, nullptr) == -1){
		exit_library(1);
	}
	fprintf(stderr, ERROR_2);
	return -1;
}


int uthread_get_tid(){
	return running_thread->get_id();
}

int uthread_get_total_quantums(){
	return counter;
}

int uthread_get_quantums(int tid){

	/*input check*/
	if(tid < 0 || tid >= (int)thread_pool.size())
	{
		fprintf(stderr, ERROR_2);
		return -1;
	}

	if (!(thread_pool[tid]->get_terminated()))
	{
		return thread_pool[tid]->get_quantum_counter();
	}

	fprintf(stderr, ERROR_2);
	return -1;
}
