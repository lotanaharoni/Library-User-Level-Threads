
#ifndef EX2_THREAD_H
#define EX2_THREAD_H


#include <setjmp.h>
#include "uthreads.h"

#define READY 1 /* ready state */
#define RUNNING 2 /* running state */
#define BLOCKED 3 /* blocked state */
typedef unsigned long address_t;


/**
 * Thread - a Thread class
 */
class Thread {

private:

	/**
	 * the thread's id
	 */
	int _id;

	/**
	 *  the thread's state
	 */
	int _state;

	/**
	 *  the thread's priority
	 */
	int _priority;

	/**
	 *  the thread's quantum counter
	 */
	int _quantum_counter;

	/**
	 *  the thread's function
	 */
	void (*_f)();

	/**
	 *  the thread's stack
	 */
	char* _stack;

	/**
	 *  thread termination flag
	 */
	bool _terminated;

	/**
	 *  thread pc pointer.
	 */
	address_t _pc;

	/**
	 *  thread sp pointer.
	 */
	address_t _sp;

	/**
 	 *  thread buff.
 	 */
	sigjmp_buf _env;

public:

	/**
	 * a constructor of a Thread object
	 * @param id - the thread's id
	 * @param priority - the thread's priority
	 * @param f - the thread's function
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
	 * adds 1 to the counter.
	 */
	void increment_counter();

	/**
	* @return the Thread's termination flag
	*/
	bool get_terminated() const;

	/**
	 * flags this thread as terminated.
	 */
	void set_terminated();

	/**
	 * returns the envelop with the data for
	 * sigjump
	 */
	sigjmp_buf& get_env();


	/**
	 * Thread destructor
	 */
	~Thread();

};


#endif //EX2_THREAD_H
