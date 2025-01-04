# Lock-Free-Data-Structures
This repository contains my implementations of some lock free data structures, as well as tests and benchmarks for comparing them with non-lock-free structures.

The project can be logically divided in two sections. The first section covers implementation of thread-safe data structures with locks. The secnond section covers different approached on implementation of lock-free data structures. Each section includes 3 parts:

A. Source code
B. Tests
C. Benchmarks

The project contains the comparison of different implementations using the provided benchmarks.

Let us discuss the first part. There are two main data structures that might be interesting for implementation:

1. Queue
2. Stack

We will implement two version of queue: 

1.1 Locknig in queue from the stadart library;
1.2 Queue as a linked list with separate locks for head and tail

The difference between the two is related to the level of concurrency that both of them are able to use.

The second section consists of 3 different implementation of queue for multithreaded environments:

2.1 SPSP unbounded queue
2.2 MPSC unbounded queue
2.3 MPMC unbounded queue
2.4 MPMC bounded queue


