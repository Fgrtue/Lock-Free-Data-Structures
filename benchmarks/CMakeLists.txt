cmake_minimum_required(VERSION 3.8)


add_executable(bench_lock_std_queue bench_lock_std_queue.cpp)

target_link_libraries(bench_lock_std_queue 
    PRIVATE
        LockFree                 
        benchmark::benchmark_main 
)

add_executable(bench_lock_fine_queue bench_lock_fine_queue.cpp)

target_link_libraries(bench_lock_fine_queue 
    PRIVATE
        LockFree                 
        benchmark::benchmark_main 
)

add_executable(bench_lock_free_spsc_queue bench_lock_free_spsc_queue.cpp)

target_link_libraries(bench_lock_free_spsc_queue 
    PRIVATE
        LockFree  
        benchmark::benchmark_main 
)

add_executable(bench_lock_free_spmc_queue bench_lock_free_spmc_queue.cpp)

target_link_libraries(bench_lock_free_spmc_queue 
    PRIVATE
        LockFree  
        atomic               
        benchmark::benchmark_main 
)