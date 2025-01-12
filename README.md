# Lock-Free-Data-Structures
This repository contains my implementations of some lock-free data structures, as well as tests and benchmarks for comparing them with each other and non-lock-free structures. 

The central part of the project is related to the implementation of different queues. The BONUS part is related to the implementation of a stack.

Let's first look at the main folders:

1. `/include` - contains the code itself. All the data structures are template ones and, therefore are written in main.
2. `/test` - contains tests for each data structure.
3. `/bechmarks` - contains benchmarks for each data structure.


## How to build

First, you have to get frameworks for runing tests and benchmarks. I used `googletest`(https://github.com/google/googletest) and `googlebenchmark` (https://github.com/google/benchmark). Add them to the repository that you pulled, build them and don't forget to write correct names in `CMakeLists.txt` in the root of the repository.

```
# 7. Add the google test subdirectory
#   and test subdirectory
add_subdirectory(googletest)

# 8. Add the google bencmark subdirectory
#   and benchmark subdirectory
add_subdirectory(benchmark) 
```

Once you have google test and google benchmark you can follow:

```
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release .. // for debug mode use -DCMAKE_BUILD_TYPE=Debug flag
```

These commands should allow you to start making the files, which are:

1. `test_lock_std_queue`
2. `test_lock_fine_queue`
3. `test_lock_free_spsc_queue`
4. `test_lock_free_spmc_queue`
5. `test_lock_free_mpmpc_bounded_queue`
6. `test_lock_std_stack` (BONUS!)
7. `test_lock_free_stack` (BONUS!)

The same files you can run with `bench` instead of `test` to have the benchmarks.

For tests, it is recommended to use `Debug` mode. For benchmarks `Release` should work better.

## What's inside

Let us take a closer (but not too close) look at the `include` folder. It contains:

1. Two implementations of non-lock-free queues
  1) Queue that uses `std::queue` as an underlying data structure
  2) Queue with better locking techniques, allowing more concurrency
2. Three implementations of lock-free queues
  1) **SPSC** queue that is incredibly quick, but is guaranteed to work only with one thread per operation
  2) **SPMC** queue that is quick and can handle multiple producers threads
  3) **MPMC** queue that is also very fast, but restricted in the number of elements that it can store
3. BONUS implementation of non-lock-free and lock-free stack
   - The only reason for them to be called bonus is that they are not guaranteed to work under any concurrency
   load. They are the result of a partially successful endeavor into the hazard pointers technique. The implementation
   follows the key guidelines of hazard pointers, however, experiences data race in the case when the number of threads for
   consumption exceeds 4.
4. Better-don't-test-it **MPSC** queue
   - This is a skeleton in the closet of this project. The ideas for the implementation of lock-free queues were taken from A.Williams
   "Concurrency in Action" book. This implementation follows all the guidelines mentioned in the reference counting technique that
   was described in `7.2.6` section in the paragraph that is called *HANDLING MULTIPLE THREADS IN PUSH()*. Even though I checked the
   details of the implementation several times to make sure that I was following all the key steps, I was experiencing a data race starting
   from `2` producers and `3` consumers.

As mentioned above, two well-known techniques were applied in order to write the some aforementioned lock-free data structures:
- Reference counting (used in **SPMC** queue)
- Hazard pointers (used in **lock-free-stack**)

It must be said that the **SPMC** queue uses an atomic structure that contains a pointer and integer. Therefore, the size of this structure
is around `96` bits, and therefore cannot be atomic on some architectures. Unfortunately, when I was testing it, it was not atomic. Therefore,
the benchmark results are not that exciting. Speaking of which...

## Results



   
