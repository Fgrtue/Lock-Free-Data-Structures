cmake_minimum_required(VERSION 3.8)


add_executable(test_lock_std_queue test_lock_std_queue.cpp)

target_link_libraries(test_lock_std_queue PRIVATE
    gtest_main
    LockFree
)