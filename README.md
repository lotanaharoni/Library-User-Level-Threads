<h1> Library User-Level Threads</h1>
<p>Include the library and use it according to the package's public interface</p>

## Table of Contents

1. [Language](#Language)
2. [Introduction](#introduction)
3. [Setup](#setup)
4. [Library functions](#Library functions)
5. [Supported OS](#supported-os)
6. [Run your tests in CI](#Run-your-tests-in-CI)
7. [Questions & Debugging & Advanced Operations](#Questions-&-Debugging-&-Advanced-Operations)
8. [Internal tools](#Internal tools)

---

## Language

This static library is written in C++

## Introduction

This is a static library, that creates and manages user-level threads.<br>
The library implements Round-Robin scheduling alghorithm.<br>
Each thread can be in one of the following states: RUNNING, BLOCKED and READY.

### Benefits

- The user can create, block, resume and terminate threads.
- The library supports different threads with different priorities, high priority threads will<br>
  get more time in the CPU when their turn arrive.

## Setup

Include the 'uthreads.h' header


## Library functions

```typescript
import { subscribe } from 'k8test'
import * as k8s from '@kubernetes/client-node' // you don't need to install it

export enum SingletonStrategy {
  manyInAppId = 'many-in-app-id',
  oneInNamespace = 'one-in-namespace',
  oneInAppId = 'one-in-app-id',
}

export type ContainerOptions = Omit<k8s.V1Container, 'name' | 'image' | 'ports'>

await subscribe({
  imageName: string
  postfix?: string
  appId?: string
  singletonStrategy?: SingletonStrategy
  imagePort: number
  containerOptions?: ContainerOptions  // for mounting and any other options
  namespaceName?: string
  isReadyPredicate?: (
    deployedImageUrl: string,
    deployedImageIp: string,
    deployedImagePort: number,
  ) => Promise<unknown>
})
```

## Run your tests in CI

```yaml
- name: install k8s
  uses: engineerd/setup-kind@v0.4.0
- run: yarn run your-tests
```

Thats it.

- I have a more advanced setup to test docker-images of other sub-packages of this repository.

## Supported OS

I'm developing on linux and macOS, and the library was tested on linux.

## Questions & Debugging & Advanced Operations

> How do I manually remove all the tests and k8test resources from my k8s cluster?

```bash
yarn k8test delete-k8test-resources
```

> How do I listen to stdout & stderr of a specific image?

work in progress. hold on. for now, you can manually search the container you need to attach to using kubectl cli, the app-id and namespace (which is k8test).

## Internal tools


- The library used system calls like 'sigsetjmp', 'siglongjmp' and 'sigprocmask'
- The following headers are included in the library: 'signal.h' and 'sys/time.h'
    
