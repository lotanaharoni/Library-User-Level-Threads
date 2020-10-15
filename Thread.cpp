#include <signal.h>
#include "Thread.h"
#ifdef __x86_64__
/* code for 64 bit Intel arch */

#define JB_SP 6
#define JB_PC 7

/* A translation is required when using an address of a variable.
   Use this as a black box in your code. */
address_t translate_address(address_t addr)
{
	address_t ret;
	asm volatile("xor    %%fs:0x30,%0\n"
				 "rol    $0x11,%0\n"
	: "=g" (ret)
	: "0" (addr));
	return ret;
}

#else
/* code for 32 bit Intel arch */

#define JB_SP 4
#define JB_PC 5

/* A translation is required when using an address of a variable.
   Use this as a black box in your code. */
address_t translate_address(address_t addr)
{
	address_t ret;
	asm volatile("xor    %%gs:0x18,%0\n"
				 "rol    $0x9,%0\n"
	: "=g" (ret)
	: "0" (addr));
	return ret;
}

#endif

Thread::Thread(int new_id, int new_p, void(*f)()) : _id(new_id), _priority(new_p), _f(f) {
	_state = READY;
	_quantum_counter = 0;
	_terminated = false;
	_stack = new char[STACK_SIZE];
	_sp = (address_t)_stack + STACK_SIZE - sizeof(address_t);
	_pc = (address_t)f;
	sigsetjmp(_env, 1);
	(_env->__jmpbuf)[JB_SP] = translate_address(_sp);
	(_env->__jmpbuf)[JB_PC] = translate_address(_pc);
	sigemptyset(&_env->__saved_mask);

}


int Thread::get_id() const {
	return _id;
}


int  Thread::get_priority() const {
	return _priority;
}


void  Thread::set_priority(const int newP) {
	_priority = newP;
}


int  Thread::get_state() const {
	return _state;
}


void  Thread::set_state(const int newS) {
	_state = newS;
	if (_state == RUNNING){
		increment_counter();
	}
}


int  Thread::get_quantum_counter() const {
	return _quantum_counter;
}


void  Thread::increment_counter() {
	_quantum_counter++;
}


bool Thread::get_terminated() const{
	return _terminated;
}

void Thread::set_terminated(){
	_terminated = true;
}

sigjmp_buf& Thread::get_env(){
	return _env;
}


Thread::~Thread()
{
	delete []_stack;
}