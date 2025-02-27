# Lock-Free-Data-Structures
This repository contains my implementations of some lock-free data structures, as well as tests and benchmarks for comparing them with each other and non-lock-free structures. 

The central part of the project is related to the implementation of different queues. The BONUS part is related to the implementation of a stack.

Let's first look at the main folders:

1. `/include` - contains the code itself. All the data structures are template ones and, therefore are written in main.
2. `/test` - contains tests for each data structure.
3. `/bechmarks` - contains benchmarks for each data structure.


## How to build

First, you have to get frameworks for running tests and benchmarks. I used `googletest`(https://github.com/google/googletest) and `googlebenchmark` (https://github.com/google/benchmark). GoogleTest will be added automatically. To add GoogleBenchmark use the following steps that are provided at GoogleBenchmark github:

```
# Check out the library.
$ git clone https://github.com/google/benchmark.git
# Go to the library root directory
$ cd benchmark
# Make a build directory to place the build output.
$ cmake -E make_directory "build"
# Generate build system files with cmake, and download any dependencies.
$ cmake -E chdir "build" cmake -DBENCHMARK_DOWNLOAD_DEPENDENCIES=on -DCMAKE_BUILD_TYPE=Release ../
# or, starting with CMake 3.13, use a simpler form:
# cmake -DBENCHMARK_DOWNLOAD_DEPENDENCIES=on -DCMAKE_BUILD_TYPE=Release -S . -B "build"
# Build the library.
$ cmake --build "build" --config Release
```

Once you have google benchmark you can follow:

```
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release .. // for debug mode use -DCMAKE_BUILD_TYPE=Debug flag
cmake --build .
```

The target files for testing are:

1. `test_lock_std_queue`
2. `test_lock_fine_queue`
3. `test_lock_free_spsc_queue`
4. `test_lock_free_spmc_queue`
5. `test_lock_free_mpmpc_bounded_queue`
6. `test_lock_std_stack` (BONUS!)
7. `test_lock_free_stack` (BONUS!)

The same files you can run with `bench` instead of `test` to have the benchmarks.

For tests, it is recommended to use `Debug` mode. For benchmarks `Release` would be more appropriate.

## What's inside

Let us take a closer (but not too close) look at the `include` folder. It contains:

1. Two implementations of non-lock-free queues
  - Queue that uses `std::queue` as an underlying data structure
  - Queue with better locking techniques, allowing more concurrency
2. Three implementations of lock-free queues
  - **SPSC** queue that is incredibly quick, but is guaranteed to work only with one thread per operation
  - **SPMC** queue that is quick and can handle multiple producer threads
  - **MPMC** queue that is also very fast, but restricted in the number of elements that it can store
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

As mentioned above, two well-known techniques were applied in order to write the some of the aforementioned lock-free data structures:
- Reference counting (used in **SPMC** queue)
- Hazard pointers (used in **lock-free-stack**)

It must be said that the **SPMC** queue uses an atomic structure that contains a pointer and integer. Therefore, the size of this structure
is around `96` bits, and therefore cannot be atomic on some architectures. Unfortunately, when I was testing it, it was not atomic. Therefore,
the benchmark results are not that exciting. Speaking of which...

## Results

The results of benchmarks are written in the folder `results_benchmarks`. There are several comparisons that might be interesting. 

#### Lock-based queues: std and fine-grained

- **Std queue**
![image](https://github.com/user-attachments/assets/d28c88cd-3de8-42bf-bf6b-2f246c9a7b86)

- **Fine-grained queue**

![image](https://github.com/user-attachments/assets/9f22ac93-b6a5-4c49-8dc4-aabe7e3ebee7)

**Conclusion:** Some improvement is seen at MPMC with `8` threads in total. This means, `4` threads for producers and `4` threads for consumers. But even in this case the improvement was not that significant.

#### Std queue vs Lock-free SPSC queue

- **Std queue**
   
![image](https://github.com/user-attachments/assets/60ca8bf7-da44-407f-9551-8e38c433a94d)

- **SPSC queue**

![image](https://github.com/user-attachments/assets/0a7ecb0d-4fde-4f2f-ad50-0357b8e4864a)

**Conclusion:** May the naming in `std` queue benchmark not confuse you. This is exactly the same test as we did in `SPSC` queue. Even though the results of push are not that encouraging, pop and push/pop test show outstanding speedup.

#### Std queue vs Lock-free SPMC queue

- **Std queue**

![image](https://github.com/user-attachments/assets/30bfcf63-0b9c-457e-a901-c1868caaf1d4)

- **SPMC queue**

![image](https://github.com/user-attachments/assets/dd678c99-6f8f-49c5-81f1-42b60c3961ff)

**Conclusion:** Here we have to stress that this is not a fair test, due to the fact that the atomic changes inside `SPMC` queue are *not lock-free*. Therefore, both of the queues are in fact non-lock-free and don't show significant improvement in the benchmarks.

#### Std queue vs Lock-free MPMC queue

- **Std queue**

![image](https://github.com/user-attachments/assets/d71910c1-5451-4490-a7fc-a790d718291c)

- **MPMC queue**

![image](https://github.com/user-attachments/assets/3669c487-2159-4d2c-baca-5fdec16844a2)

**Conclusion:** This is the finest example of a lock-free structure of this project. `MPMC` bounded queue kicks the insides out of the `std` queue in all the benchmarks.

#### BONUS Std stack vs Lock-free stack

- **Std stack**

![image](https://github.com/user-attachments/assets/fe3b2c12-fefe-484b-bb76-163a50c4329b)

- **Lock-free stack**

![image](https://github.com/user-attachments/assets/0a6f52b4-7993-4f8e-838d-cdec43c66c22)


**Conclusion:** Unfortunately this one isn't much better than the lock-free `SPMC` queues. One of the issues (which is also the case for other queues) is the usage of a single-threaded memory allocator. I decided to leave this part of research into the details of the implementation of multithreaded memory allocators for the future.

## Some words about tests and benchmarks

In the tests, we went from less concurrent to more concurrent test cases. For almost all the tests we did a high concurrent test with `50` threads pushing, `50` threads popping, and `1` million elements. We also ensured that our data structures are exception-safe but testing them with elements that throw exceptions in copy and move constructors. In benchmarks, we measured the latency of each operation separately, as well as the total time for `100'000 * NumberOfThreads` pushes or pops. Another measurement is related to the throughput of data structure, which is specified in the `items_per_second` column.
