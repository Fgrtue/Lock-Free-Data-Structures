#include "../include/lock-free-mpmc-bounded-queue.hpp"

#include <gtest/gtest.h>
#include <iostream>
#include <thread>
#include <vector>
#include <unordered_set>
#include <random>
#include <chrono>
#include  <stdexcept>
#include <memory>

/*
1. Basic Functionality

    - Elements pushed into the queue can be popped
    in the correct order
    - Ensure that empty returns true for new queue
    - Try successful/uncuccessful try_pop operations

2. Concurrent Access Tests

    - Multiple Consumers
        -> Multiple threads concurrently pop elements from the
        queue
        Expect: each element is consumed exactly once

3. Stress Tests
    - Push and Pop a large number of elements concurrently to test the
    queue under heavy worklads

4. Exception Safety Tests
    - Simulate exeptions during push or pop operations to ensure that
    the queue remains in a consistent state and no deadlock happens
*/

// // 1. Single thread, empty
TEST(Basic, Empty) {
    lock_free_mpmc_bounded_queue<int> q(1024);
    EXPECT_TRUE(q.empty());
}

// // 2. Single thread, Push and then
// //  try pop with ptr
TEST(Basic, Push_TryPopPtr) {

    lock_free_mpmc_bounded_queue<int> q(1024);

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

    lock_free_mpmc_bounded_queue<int> q(1024);

    auto ptr = q.pop();
    EXPECT_FALSE(ptr);
}


// /*
// ###################################################

//             SHARED_PTR Functionality

// ###################################################
// */

// 5. Single Producer, Single Consumer
//    -> in the end we must have all the elemens in
//          the same order as we pushed
TEST(Concurrent, SPSC) {

    lock_free_mpmc_bounded_queue<int> q(1024);

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
            auto res = q.pop();
                if (res) {
                    values[i] = *res;
                } else {
                    --i;
                }
        }
    });

    for (int i = 0; i < concurrency_level; ++i) {
        threads[i].join();
    }

    for (int i = 0; i < n; ++i) {
        EXPECT_EQ(i, values[i]);
    }
}

// 6. Single Producer, Multiple Consumer
//    -> in the end we must have all the elemens in
//       some order
TEST(Concurrent, SPMC) {

    lock_free_mpmc_bounded_queue<int> q(1024);

    std::vector<std::thread> threads;
    int concurrency_level = 9;
    int n = 8000;
    threads.emplace_back( [&](){
        for (int i = 0; i < n; ++i) {
            while(!q.push(i));
        }
    });

    std::vector<std::atomic<bool>> values(n);
    for (int i = 0; i < concurrency_level - 1; ++i) {
        threads.emplace_back([&]() {
            for (int j = 0; j < n / (concurrency_level - 1); ++j) {

                std::unique_ptr<int> res; 
                while((res = q.pop()) == nullptr);
                values[*res].store(true);
            }
        });
    }

    for (int i = 0; i < concurrency_level; ++i) {
        threads[i].join();
    }

    for (int i = 0; i < n; ++i) {
        // EXPECT_TRUE(values[i].load(std::memory_order_relaxed)) << "i= " << i << "\n";
    }
}

// // // 7. Multiple Producers, Single Consumer
// // //    -> in the end we have all the elements that we pushed

TEST(Concurrent, MPSC) {

    lock_free_mpmc_bounded_queue<int> q(1024);
    std::vector<std::thread> threads;
    int concurrency_level = 9;
    int n = 8000;
    for (int i = 0; i < (concurrency_level - 1); ++i) {
        threads.emplace_back([i, &q, n, &concurrency_level]() {
            int beg = i * (n / (concurrency_level - 1));
            int end = (i + 1) * (n / (concurrency_level - 1));
            for (int j = beg; j < end; ++j) {
                while(!q.push(j));
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

// 8. Multiple Producres, Multiple Consumers
//    -> in the end we have all the elements that we pushed
TEST(Concurrent, MPMC_PTR) {

    lock_free_mpmc_bounded_queue<int> q(1024);

    std::vector<std::thread> threads;
    int concurrency_level = 8;
    int n = 1200;
    for (int i = 0; i < (concurrency_level / 2); ++i) {
        threads.emplace_back([i, &q, n, concurrency_level]() {
            int beg = i * (n / (concurrency_level / 2));
            int end = (i + 1) * (n / (concurrency_level / 2));
            for (int j = beg; j < end; ++j) {
                while(!q.push(j));
            }
        });
    }

    std::vector<std::atomic<bool>> values(n);
    for (int i = 0; i < (concurrency_level / 2); ++i) {
        threads.emplace_back([n, &q, &values, concurrency_level]() {
            for (int j = 0; j < n / (concurrency_level / 2); ++j) {
                std::unique_ptr<int> res;
                while((res = q.pop()) == nullptr);
                values[*res].store(true, std::memory_order_relaxed);
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

// 9. Multiple Producres, Multiple Consumers
//    -> in the end we have all the elements that we pushed

TEST(Stress, HighMPMC_PTR) {

    lock_free_mpmc_bounded_queue<int> q(500'000);

    std::vector<std::thread> threads;
    int number_of_producers = 50;
    int number_of_consumers = 50;
    int n = 1'000'000;
    for (int i = 0; i < number_of_producers; ++i) {
        threads.emplace_back([i, &q, n, number_of_producers]() {
            int beg = i * (n / number_of_producers);
            int end = (i + 1) * (n / number_of_producers);
            for (int j = beg; j < end; ++j) {
                while(!q.push(j));

            }
        });
    }

    std::vector<std::atomic<bool>> values(n);
    for (int i = 0; i < number_of_consumers; ++i) {
        threads.emplace_back([n, &q, &values, number_of_consumers]() {
            for (int j = 0; j < n / number_of_consumers; ++j) {
                std::unique_ptr<int> res;
                while((res = q.pop()) == nullptr);
                values[*res].store(true, std::memory_order_relaxed);
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
TEST(Stress, RandMPMC_PTR) {

    lock_free_mpmc_bounded_queue<int> q(10'000);


    std::vector<std::thread> threads;
    int number_of_producers = 25;
    int number_of_consumers = 25;
    int n = 50'000;

    for (int i = 0; i < number_of_producers; ++i) {
        threads.emplace_back([i, &q, n, number_of_producers]() {
            std::mt19937 gen(std::random_device{}());
            std::uniform_int_distribution<int> dist(0, 10);
            int beg = i * (n / number_of_producers);
            int end = (i + 1) * (n / number_of_producers);
            for (int j = beg; j < end; ++j) {
                while(!q.push(j));
                std::this_thread::sleep_for(std::chrono::milliseconds(dist(gen)));
            }
        });
    }

    std::vector<std::atomic<bool>> values(n);

    for (int i = 0; i < number_of_consumers; ++i) {
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
    }

    for (int i = 0; i < (number_of_consumers + number_of_producers); ++i) {
        threads[i].join();
    }

    for (int i = 0; i < n; ++i) {
        EXPECT_TRUE(values[i].load(std::memory_order_relaxed)) << "i= " << i << "\n";
    }
}

// 10. Exception handelling
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
            throw std::runtime_error(std::to_string(i_));
        }
    }

    ExeptInt(ExeptInt&& other) noexcept(false)
    : i_(other.i_), fail_(other.fail_)
    {
        if (fail_) {
            throw std::runtime_error(std::to_string(i_));
        }
    }

    ExeptInt& operator= (const ExeptInt& other) {

        if (fail_) {
            throw std::runtime_error(std::to_string(i_));
        }

        i_ = other.i_;
        fail_ = other.fail_;

        return *this;
    }

    ExeptInt& operator= (ExeptInt&& other) noexcept(false)
    {

        if (fail_) {
            throw std::runtime_error(std::to_string(i_));
        }

        i_ = other.i_;
        fail_ = other.fail_;

        return *this;
    }
 
    int i_;
    bool fail_;
};

TEST(Exception, MPMC_PTR) {

    lock_free_mpmc_bounded_queue<ExeptInt> q(10'000);

    std::vector<std::thread> threads;
    int concurrency_level = 16;
    int n = 160'000;
    std::vector<std::atomic<bool>> values(n);

    for (int i = 0; i < (concurrency_level / 2); ++i) {
        threads.emplace_back([i, &q, n, concurrency_level]() {
            std::mt19937 gen(std::random_device{}());
            std::uniform_int_distribution<int> dist(1, 6);
            int beg = i * (n / (concurrency_level / 2));
            int end = (i + 1) * (n / (concurrency_level / 2));
            for (int j = beg; j < end; ++j) {
                ExeptInt num(j, dist(gen) / 6);
                try {
                    while(!q.push(num));
                } catch (const std::exception& e) {
                    ExeptInt num2(j, false);
                    while(!q.push(num2));
                }
            }
        });
    }

    for (int i = 0; i < (concurrency_level / 2); ++i) {
        threads.emplace_back([n, &q, &values, concurrency_level]() {

            for (int j = 0; j < n / (concurrency_level / 2); ++j) {
                std::shared_ptr<ExeptInt> res;
                while((res = q.pop()) == nullptr);
                values[res->i_].store(true, std::memory_order_relaxed);
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


