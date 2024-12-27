cmake_minimum_required(VERSION 3.8)

set(TEST_EXEC_NAME Tests)

set (SOURCES
        Tests_LockSTDQ.cpp
)

add_executable(${TEST_EXEC_NAME} ${SOURCES})
target_link_libraries(${TEST_EXEC_NAME} PRIVATE
    gtest_main
    LockFree
)

add_test(
    NAME ${TEST_EXEC_NAME}
    COMMAND ${TEST_EXEC_NAME}
)