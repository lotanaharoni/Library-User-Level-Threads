<h1> Library User-Level Threads</h1>
<p>Include the library and use it according to the package's public interface</p>

## Table of Contents

1. [Language](#Language)
2. [Introduction](#introduction)
3. [Setup](#setup)
4. [Library functions](#Library-functions)
5. [Supported OS](#supported-os)
6. [Internal tools](#Internal-tools)

---

## Language

This static library is written in C++.
<br>

## Introduction

This is a static library, that creates and manages user-level threads.<br>
The library implements Round-Robin scheduling alghorithm.<br>
Each thread can be in one of the following states: RUNNING, BLOCKED and READY.
<br>

### Benefits

- The user can create, block, resume and terminate threads.
- The library supports different threads with different priorities, high priority threads will<br>
  get more time in the CPU when their turn arrive.

## Setup

Include the 'uthreads.h' header
<br>

## Library functions

```typescript
int uthread_init(int *quantum_usecs, int size)
/**
 * This function initializes the thread library.
 */

int uthread_spawn(void (*f)(void), int priority)
/**
 * This function creates a new thread, whose entry point is the function f with the signature void f(void).
 */

int uthread_change_priority(int tid, int priority)
/**
 * This function initializes the thread library.
 */

int uthread_terminate(int tid)
/**
 * This function changes the priority of the thread with ID tid.
 */

int uthread_block(int tid)
/**
 *  This function blocks the thread with ID tid.
 */

int uthread_resume(int tid)
/**
 * This function resumes a blocked thread with ID tid.
 */

```

## Supported OS

I'm developing on linux and macOS, and the library was tested on linux.

## Internal tools


- The library used system calls like 'sigsetjmp', 'siglongjmp' and 'sigprocmask'
- The following headers are included in the library: 'signal.h' and 'sys/time.h'
    
