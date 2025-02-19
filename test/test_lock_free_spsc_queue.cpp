#include "lock-free-spsc-queue.hpp"

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
    lock_free_spsc_queue<int> q;
    EXPECT_TRUE(q.empty());
}

// 2. Single thread, Push and then
//  try pop with value
TEST(Basic, Push_TryPopVal) {

    lock_free_spsc_queue<int> q;

    q.push(1);
    q.push(2);
    q.push(3);
    
    int val;
    EXPECT_TRUE(q.pop(val));
    EXPECT_EQ(1, val);
    EXPECT_TRUE(q.pop(val));
    EXPECT_EQ(2, val);
    EXPECT_TRUE(q.pop(val));
    EXPECT_EQ(3, val);
}

// 3. Single thread, Push and then
//  try pop with ptr
TEST(Basic, Push_TryPopPtr) {

    lock_free_spsc_queue<int> q;

    q.push(1);
    q.push(2);
    q.push(3);
    
    auto ptr = q.pop();
    ASSERT_TRUE(ptr);
    EXPECT_EQ(1, *ptr);

    ptr = q.pop();
    ASSERT_TRUE(ptr);
    EXPECT_EQ(2, *ptr);

    ptr = q.pop();
    ASSERT_TRUE(ptr);
    EXPECT_EQ(3, *ptr);
}

// 4. Single thread, unsuccessful pop
TEST(Basic, Unsussesful_Pop) {

    lock_free_spsc_queue<int> q;
    int val;
    EXPECT_FALSE(q.pop(val));
    EXPECT_FALSE(q.pop());
}

// 5. Single Producer, Single Consumer
//    -> in the end we must have all the elemens in
//          the same order as we pushed
TEST(Concurrent, SPSC) {

    lock_free_spsc_queue<int> q;
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
            while(!q.pop(values[i]));
        }
    });

    for (int i = 0; i < concurrency_level; ++i) {
        threads[i].join();
    }

    for (int i = 0; i < n; ++i) {
        EXPECT_EQ(i, values[i]);
    }
}

// 9. Large number of push and pop operations
//    -> increase number of threas beyond typical (Note: here only 1 possible)
//    -> use very high N (1 000 000)
TEST(Stress, HighSPSC) {

    lock_free_spsc_queue<int> q;
    std::vector<std::thread> threads;
    int number_of_producers = 1;
    int number_of_consumers = 1;
    int n = 1'000'000;
    for (int i = 0; i < number_of_producers; ++i) {
        threads.emplace_back([i, &q, n, number_of_producers]() {
            int beg = i * (n / number_of_producers);
            int end = (i + 1) * (n / number_of_producers);
            for (int j = beg; j < end; ++j) {
                q.push(j);
            }
        });
    }

    std::vector<std::atomic<bool>> values(n);
    for (int i = 0; i < number_of_consumers; ++i) {
        threads.emplace_back([n, &q, &values, number_of_consumers]() {
            for (int j = 0; j < n / number_of_consumers; ++j) {
                int val;
                while(!q.pop(val));
                values[val].store(true, std::memory_order_relaxed);
            }
        });
    }

    for (int i = 0; i < (number_of_consumers + number_of_producers); ++i) {
        threads[i].join();
    }

    for (int i = 0; i < n; ++i) {
        EXPECT_TRUE(values[i].load(std::memory_order_relaxed)) << "i= " << i << "\n";
    }
}

// 10. Random delays
//     -> Introduce random sleep interval in producer 
//          and consumer threads
TEST(Stress, RandSPSC) {

    lock_free_spsc_queue<int> q;
    std::vector<std::thread> threads;
    int number_of_producers = 1;
    int number_of_consumers = 1;
    int n = 50000;

    threads.emplace_back([&q, n]() {
        std::mt19937 gen(std::random_device{}());
        std::uniform_int_distribution<int> dist(0, 10);
        for (int j = 0; j < n; ++j) {
            q.push(j);
            std::this_thread::sleep_for(std::chrono::milliseconds(dist(gen)));
        }
    });

    std::vector<std::atomic<bool>> values(n);

    threads.emplace_back([n, &q, &values]() {
        std::mt19937 gen(std::random_device{}());
        std::uniform_int_distribution<int> dist(0, 10);
        for (int j = 0; j < n; ++j) {
            int val;
            while(!q.pop(val));
            values[val].store(true, std::memory_order_relaxed);
            std::this_thread::sleep_for(std::chrono::milliseconds(dist(gen)));
        }
    });

    for (int i = 0; i < (number_of_consumers + number_of_producers); ++i) {
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

TEST(Exception, SPSC) {

    lock_free_spsc_queue<ExeptInt> q;
    std::vector<std::thread> threads;
    int concurrency_level = 2;
    int n = 1200;

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
    threads.emplace_back([n, &q, &values]() {
        std::mt19937 gen(std::random_device{}());
        std::uniform_int_distribution<int> dist(1, 6);
        for (int j = 0; j < n; ++j) {
            ExeptInt val(0, dist(gen) / 6);
            try {
                while(!q.pop(val));
                values[val.i_].store(true, std::memory_order_relaxed);
            } catch (const std::exception& e) {
                ExeptInt val2(0, 0);
                while(!q.pop(val2));
                values[val2.i_].store(true, std::memory_order_relaxed);
            }
        }
    });

    for (int i = 0; i < concurrency_level; ++i) {
        threads[i].join();
    }

    for (int i = 0; i < n; ++i) {
        EXPECT_TRUE(values[i].load(std::memory_order_relaxed)) << "i= " << i << "\n";
    }
}

/*
###################################################

            SHARED_PTR Functionality

###################################################
*/

// 12. Single Producer, Single Consumer
//    -> in the end we must have all the elemens in
//          the same order as we pushed
TEST(Concurrent, SPSC_PTR) {

    lock_free_spsc_queue<int> q;
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
            std::shared_ptr<int> res = nullptr;
            while(!res) {
                res = q.pop();
            }
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

// 16. Large number of push and pop operations
//    -> increase number of threas beyond typical
//    -> use very high N (1 000 000)
TEST(Stress, HighSPSC_PTR) {

    lock_free_spsc_queue<int> q;
    std::vector<std::thread> threads;
    int number_of_producers = 1;
    int number_of_consumers = 1;
    int n = 1'000'000;
    threads.emplace_back([&q, n]() {
        for (int j = 0; j < n; ++j) {
            q.push(j);
        }
    });

    std::vector<std::atomic<bool>> values(n);
    threads.emplace_back([n, &q, &values, number_of_consumers]() {
        for (int j = 0; j < n / number_of_consumers; ++j) {
            std::shared_ptr<int> res = nullptr;
            while(!res) {
                res = q.pop();
            }
            values[*res].store(true, std::memory_order_relaxed);
        }
    });

    for (int i = 0; i < (number_of_consumers + number_of_producers); ++i) {
        threads[i].join();
    }

    for (int i = 0; i < n; ++i) {
        EXPECT_TRUE(values[i].load(std::memory_order_relaxed)) << "i= " << i << "\n";
    }
}

// 17. Random delays
//     -> Introduce random sleep interval in producer 
//          and consumer threads
TEST(Stress, RandSPSC_PTR) {

    lock_free_spsc_queue<int> q;
    std::vector<std::thread> threads;
    int number_of_producers = 1;
    int number_of_consumers = 1;
    int n = 50000;

    threads.emplace_back([&q, n]() {
        std::mt19937 gen(std::random_device{}());
        std::uniform_int_distribution<int> dist(0, 10);
        for (int j = 0; j < n; ++j) {
            q.push(j);
            std::this_thread::sleep_for(std::chrono::milliseconds(dist(gen)));
        }
    });

    std::vector<std::atomic<bool>> values(n);

    threads.emplace_back([n, &q, &values, number_of_consumers]() {
        std::mt19937 gen(std::random_device{}());
        std::uniform_int_distribution<int> dist(0, 10);
        for (int j = 0; j < n / number_of_consumers; ++j) {
            std::shared_ptr<int> res = nullptr;
            while(!res) {
                res = q.pop();
            }
            values[*res].store(true, std::memory_order_relaxed);
            std::this_thread::sleep_for(std::chrono::milliseconds(dist(gen)));
        }
    });

    for (int i = 0; i < (number_of_consumers + number_of_producers); ++i) {
        threads[i].join();
    }

    for (int i = 0; i < n; ++i) {
        EXPECT_TRUE(values[i].load(std::memory_order_relaxed)) << "i= " << i << "\n";
    }
}

// 18. Exception handelling
//    -> Create a type, which in copy/move assignment/operator
//      throws exeptions with probability 1/6
//    -> try common tests with push pop to ensure that everything works

TEST(Exception, SPSC_PTR) {

    lock_free_spsc_queue<ExeptInt> q;
    std::vector<std::thread> threads;
    int concurrency_level = 2;
    int n = 1200;
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
        threads.emplace_back([n, &q, &values]() {
            std::mt19937 gen(std::random_device{}());
            std::uniform_int_distribution<int> dist(1, 6);
            for (int j = 0; j < n; ++j) {
                try {
                    std::shared_ptr<ExeptInt> res = nullptr;
                    while(!res) {
                        res = q.pop();
                    }
                    values[res->i_].store(true, std::memory_order_relaxed);
                } catch (const std::exception& e) {
                    std::shared_ptr<ExeptInt> res2 = nullptr;
                    while(!res2) {
                        res2 = q.pop();
                    }
                    values[res2->i_].store(true, std::memory_order_relaxed);
                }
            }
        });

    for (int i = 0; i < concurrency_level; ++i) {
        threads[i].join();
    }


    for (int i = 0; i < n; ++i) {
        EXPECT_TRUE(values[i].load(std::memory_order_relaxed)) << "i= " << i << "\n";
    }
}