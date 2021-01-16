
#ifndef EX2_THREAD_H
#define EX2_THREAD_H


#include <setjmp.h>
#include "uthreads.h"

#define READY 1 /* ready state */
#define RUNNING 2 /* running state */
#define BLOCKED 3 /* blocked state */
typedef unsigned long address_t;


class Thread {

private:

	int _id;

	int _state;

	int _priority;
	
	int _quantum_counter;

	void (*_f)();

	char* _stack;

	bool _terminated;

	address_t _pc;

	address_t _sp;

	sigjmp_buf _env;

public:

	/**
	 * a constructor of a Thread object
	 * @param id - The thread's id
	 * @param priority - The thread's priority
	 * @param f - The thread's function
	 */
	Thread(int id, int priority, void(*f)());


	/**
	 * @return the Thread's id
	 */
	int get_id() const;

	/**
	 * @return the Thread's priority
	 */
	int get_priority() const;

	/**
	 * sets a new priority to the thread.
	 * @param newP The threads new priority
	 */
	void set_priority(int newP);

	/**
	 * @return the Thread's state
	 */
	int get_state() const;

	/**
	 * sets a new state to the thread.
	 * @param newS
	 */
	void set_state(int newS);

	/**
	 * @return the Thread's Quantum counter
	 */
	int get_quantum_counter() const;

	/**
	 * Adds 1 to the counter.
	 */
	void increment_counter();

	/**
	* Return the Thread's termination flag
	*/
	bool get_terminated() const;

	/**
	 * Flags this thread as terminated.
	 */
	void set_terminated();

	/**
	 * Returns the envelop with the data for
	 * sigjump
	 */
	sigjmp_buf& get_env();


	/**
	 * Thread destructor
	 */
	~Thread();

};


#endif //EX2_THREAD_H
