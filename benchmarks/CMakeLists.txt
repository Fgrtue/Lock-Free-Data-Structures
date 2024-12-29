cmake_minimum_required(VERSION 3.8)


add_executable(bench_lock_std_queue bench_lock_std_queue.cpp)

target_link_libraries(bench_lock_std_queue 
    PRIVATE
        LockFree                 
        benchmark::benchmark_main 
)