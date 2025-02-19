#include "lock-free-mpsc-queue.hpp"

#include <gtest/gtest.h>
#include <iostream>
#include <thread>
#include <vector>
#include <unordered_set>
#include <random>
#include <chrono>
#include  <stdexcept>

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
    the queue remains in a consistent state and no deadlock happens
*/

// 1. Single thread, empty
TEST(Basic, Empty) {
    lock_free_mpsc_queue<int> q;
    EXPECT_TRUE(q.empty());
}

// 2. Single thread, Push and then
//  try pop with value
TEST(Basic, Push_TryPopVal) {

    lock_free_mpsc_queue<int> q;

    q.push(1);
    q.push(2);
    q.push(3);

    auto ptr1 = q.pop();
    ASSERT_TRUE(ptr1);
    EXPECT_EQ(1, *ptr1);
    auto ptr2 = q.pop();
    ASSERT_TRUE(ptr2);
    EXPECT_EQ(2, *ptr2);    
    auto ptr3 = q.pop();
    ASSERT_TRUE(ptr3);
    EXPECT_EQ(3, *ptr3);
}

// 3. Single thread, unsuccessful pop
TEST(Basic, Unsussesful_Pop) {

    lock_free_mpsc_queue<int> q;
    EXPECT_FALSE(q.pop());
}

// // 4. Single Producer, Single Consumer
// //    -> in the end we must have all the elemens in
// //          the same order as we pushed
TEST(Concurrent, SPSC) {

    lock_free_mpsc_queue<int> q;
    std::vector<std::thread> threads;
    int concurrency_level = 2;
    int n = 1000;
    threads.emplace_back( [&](){
        for (int i = 0; i < n; ++i) {
            q.push(i);
        }
    });
    std::vector<int> values(n);
    threads.emplace_back( [&](){
        for (int i = 0; i < n; ++i) {
            std::unique_ptr<int> res;
            while((res = q.pop()) == nullptr);
            values[i] = *res;
        }
    });

    for (int i = 0; i < concurrency_level; ++i) {
        threads[i].join();
    }

    for (int i = 0; i < n; ++i) {
        EXPECT_EQ(i, values[i]);
    }
}

// // 5. Single Producer, Multiple Consumers
// //    -> in the end we must have all the elements

// TEST(Concurrent, SPMC) {

//     lock_free_mpsc_queue<int> q;
//     std::vector<std::thread> threads;
//     int concurrency_level = 51;
//     int n = 500'000;
//     threads.emplace_back( [&](){
//         for (int i = 0; i < n; ++i) {
//             q.push(i);
//         }
//     });

//     std::vector<std::atomic<bool>> values(n);
//     for (int i = 0; i < (concurrency_level - 1); ++i) {
//         threads.emplace_back([&]() {
//             for (int j = 0; j < n / (concurrency_level - 1); ++j) {
//                 std::unique_ptr<int> res;
//                 while((res = q.pop()) == nullptr);
//                 values[*res].store(true, std::memory_order_relaxed);
//             }
//         });
//     }

//     for (int i = 0; i < concurrency_level; ++i) {
//         threads[i].join();
//     }

//     for (int i = 0; i < n; ++i) {
//         EXPECT_TRUE(values[i].load(std::memory_order_relaxed)) << "i= " << i << "\n";
//     }
// }

// // // 7. Multiple Producers, Single Consumer
// // //    -> in the end we have all the elements that we pushed

TEST(Concurrent, MPSC) {

    lock_free_mpsc_queue<int> q;
    std::vector<std::thread> threads;
    int concurrency_level = 33;
    int n = 3'300'000;
    for (int i = 0; i < (concurrency_level - 1); ++i) {
        threads.emplace_back([i, &q, n, &concurrency_level]() {
            int beg = i * (n / (concurrency_level - 1));
            int end = (i + 1) * (n / (concurrency_level - 1));
            for (int j = beg; j < end; ++j) {
                q.push(j);
            }
        });
    }

    std::vector<std::atomic<bool>> values(n);
    threads.emplace_back([&]() {
        for (int j = 0; j < n; ++j) {
            std::unique_ptr<int> res;
            while((res = q.pop()) == nullptr);
            values[*res].store(true, std::memory_order_relaxed);
        }
    });

    for (int i = 0; i < concurrency_level; ++i) {
        threads[i].join();
    }

    for (int i = 0; i < n; ++i) {
        EXPECT_TRUE(values[i].load(std::memory_order_relaxed)) << "i= " << i << "\n";
    }
}

// 11. Exception handelling
//    -> Create a type, which in copy/move assignment/operator
//      throws exeptions with probability 1/6
//    -> try common tests with push pop to ensure that everything works

struct ExeptInt {

    ExeptInt(int i, bool ex)
    : i_(i)
    , fail_(ex)
    {}

    ExeptInt(const ExeptInt& other)
    : i_(other.i_), fail_(other.fail_)
    {
        if (fail_) {
            throw std::runtime_error("");
        }
    }

    ExeptInt(ExeptInt&& other) noexcept(false)
    : i_(other.i_), fail_(other.fail_)
    {
        if (fail_) {
            throw std::runtime_error("");
        }
    }

    ExeptInt& operator= (const ExeptInt& other) {

        if (fail_) {
            throw std::runtime_error("");
        }

        i_ = other.i_;
        fail_ = other.fail_;

        return *this;
    }

    ExeptInt& operator= (ExeptInt&& other) noexcept(false)
    {

        if (fail_) {
            throw std::runtime_error("");
        }

        i_ = other.i_;
        fail_ = other.fail_;

        return *this;
    }
 
    int i_;
    bool fail_;
};

TEST(Exception, MPMC) {

    lock_free_mpsc_queue<ExeptInt> q;
    std::vector<std::thread> threads;
    int concurrency_level = 9;
    int n = 8000;
    threads.emplace_back([&q, n]() {
        
        std::mt19937 gen(std::random_device{}());
        std::uniform_int_distribution<int> dist(1, 6);
        for (int j = 0; j < n; ++j) {
            ExeptInt num(j, dist(gen) / 6);
            try {
                q.push(num);
            } catch (const std::exception& e) {
                ExeptInt num2(j, false);
                q.push(num2);
            }
        }
    });

    std::vector<std::atomic<bool>> values(n);
    for (int i = 0; i < (concurrency_level - 1); ++i) {
        threads.emplace_back([n, &q, &values, &concurrency_level]() {
            std::mt19937 gen(std::random_device{}());
            std::uniform_int_distribution<int> dist(1, 6);
            for (int j = 0; j < n / (concurrency_level - 1); ++j) {
                ExeptInt val(0, dist(gen) / 6);
                try {
                    std::unique_ptr<ExeptInt> res;
                    while((res = q.pop()) == nullptr);
                    values[res->i_].store(true, std::memory_order_relaxed);
                } catch (const std::exception& e) {
                    std::unique_ptr<ExeptInt> res2;
                    while((res2 = q.pop()) == nullptr);
                    values[res2->i_].store(true, std::memory_order_relaxed);
                }
            }
        });
    }

    for (int i = 0; i < concurrency_level; ++i) {
        threads[i].join();
    }

    for (int i = 0; i < n; ++i) {
        EXPECT_TRUE(values[i].load(std::memory_order_relaxed)) << "i= " << i << "\n";
    }
}
