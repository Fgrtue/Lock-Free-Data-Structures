#include "../include/lock-std-queue.hpp"

#include <gtest/gtest.h>
#include <iostream>
#include <thread>
#include <vector>

/*

1. Basic Functionality

    - Elements pushed into the queue can be popped
    in the correct order
    - Ensure that empty returns true for new queue
    - Try successful/uncuccessful try_pop operations

2. Concurrent Access Tests

    - Multiple Producers
        -> Multiple threads concurrently push elements into the
        queue
        Expect: all elements are succesfully enqueued without data loss
    - Multiple Consumers
        -> Multiple threads concurrently pop elements from the
        queue
        Expect: each element is consumed exactly once
    - Producers and Comsumers
        -> Combine multople producer and consumer threads to test the queue
        under realistic workloads

3. Stress Tests
    - Push and Pop a large number of elements concurrently to test the
    queue under heavy worklads

4. Exception Safety Tests
    - Simulate exeptions during push or pop operations to ensure that
    the queu remains in a consistent state and no deadlock happens
*/

// 1. Single thread, empty
TEST(Basic, Empty) {
    lock_std_queue<int> q;
    EXPECT_TRUE(q.empty());
}

// 2. Single thread, Push and then
//  try pop with value
TEST(Basic, Push_TryPopVal) {

    lock_std_queue<int> q;

    q.push(1);
    q.push(2);
    q.push(3);
    
    int val;
    EXPECT_TRUE(q.try_pop(val));
    EXPECT_EQ(1, val);
    EXPECT_TRUE(q.try_pop(val));
    EXPECT_EQ(2, val);
    EXPECT_TRUE(q.try_pop(val));
    EXPECT_EQ(3, val);
}

// 3. Single thread, Push and then
//  try pop with ptr
TEST(Basic, Push_TryPopPtr) {

    lock_std_queue<int> q;

    q.push(1);
    q.push(2);
    q.push(3);
    
    auto ptr = q.try_pop();
    ASSERT_TRUE(ptr);
    EXPECT_EQ(1, *ptr);

    ptr = q.try_pop();
    ASSERT_TRUE(ptr);
    EXPECT_EQ(2, *ptr);

    ptr = q.try_pop();
    ASSERT_TRUE(ptr);
    EXPECT_EQ(3, *ptr);
}

// 4. Single thread, unsuccessful pop
TEST(Basic, Unsussesful_Pop) {

    lock_std_queue<int> q;
    int val;
    EXPECT_FALSE(q.try_pop(val));
    EXPECT_FALSE(q.try_pop());
}

// 5. Single Producer, Single Consumer
//    -> in the end we must have all the elemens in
//          the same order as we pushed
TEST(Concurrent, SPSC) {

    std::vector<std::thread>

}


// 6. Single Producer, Multiple Consumers
//    -> in the end we must have all the elements

// 7. Multiple Producers, Single Consumer
//    -> in the end we have all the elements that we pushed

// 8. Multiple Producres, Multiple Consumers
//    -> in the end we have all the elements that we pushed




