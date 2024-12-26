
The project can be logically divided in two sections. The first section covers implementation of thread-safe data structures with locks. The secnond section is more broad and covers different approached on implementation of lock-free data structures. Each section includes 3 parts:

A. Source code
B. Tests
C. Benchmarks

The project contains the comparison of different implementations using the provided benchmarks.

Let us discuss the first part. There are two main data structures that might be interesting for implementation:

1. Queue
2. Hash Map

We will implement two version of queue: 

1.1 Locknig in queue from the stadart library;
1.2 Queue as a linked list.

The difference between the two is related to the level of concurrency that both of them are able to use.

The second section consists of 4 different implementation of queue for multithreaded environments:

2.1 SPSP unbounded queue
2.2 MPSC bounded queue
2.3 MPMC bounded queue


