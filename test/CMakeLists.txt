cmake_minimum_required(VERSION 3.8)


add_executable(test_lock_std_queue test_lock_std_queue.cpp)

target_link_libraries(test_lock_std_queue PRIVATE
    gtest_main
    LockFree
)

add_executable(test_lock_fine_queue test_lock_fine_queue.cpp)

target_link_libraries(test_lock_fine_queue PRIVATE
    gtest_main
    LockFree
)

add_executable(test_lock_free_spsc_queue test_lock_free_spsc_queue.cpp)

target_link_libraries(test_lock_free_spsc_queue PRIVATE
    gtest_main
    LockFree
)

add_executable(test_lock_free_spmc_queue test_lock_free_spmc_queue.cpp)

target_link_libraries(test_lock_free_spmc_queue PRIVATE
    gtest_main
    atomic
    LockFree
)

add_executable(test_lock_free_mpsc_queue test_lock_free_mpsc_queue.cpp)

target_link_libraries(test_lock_free_mpsc_queue PRIVATE
    gtest_main
    atomic
    LockFree
)


add_executable(test_lock_free_mpmc_bounded_queue test_lock_free_mpmc_bounded_queue.cpp)

target_link_libraries(test_lock_free_mpmc_bounded_queue PRIVATE
    gtest_main
    LockFree
)

add_executable(test_hazard_pointers test_hazard_pointers.cpp)

target_link_libraries(test_hazard_pointers PRIVATE
    gtest_main
    LockFree
)

add_executable(test_lock_free_stack test_lock_free_stack.cpp)

target_link_libraries(test_lock_free_stack PRIVATE
    gtest_main
    LockFree
)