#include "lock-fine-queue.hpp"

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
    lock_fine_queue<int> q;
    EXPECT_TRUE(q.empty());
}

// 2. Single thread, Push and then
//  try pop with value
TEST(Basic, Push_TryPopVal) {

    lock_fine_queue<int> q;

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

    lock_fine_queue<int> q;

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

    lock_fine_queue<int> q;
    int val;
    EXPECT_FALSE(q.try_pop(val));
    EXPECT_FALSE(q.try_pop());
}

// 5. Single Producer, Single Consumer
//    -> in the end we must have all the elemens in
//          the same order as we pushed
TEST(Concurrent, SPSC) {

    lock_fine_queue<int> q;
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
            q.wait_and_pop(values[i]);
        }
    });

    for (int i = 0; i < concurrency_level; ++i) {
        threads[i].join();
    }

    for (int i = 0; i < n; ++i) {
        EXPECT_EQ(i, values[i]);
    }
}

// 6. Single Producer, Multiple Consumers
//    -> in the end we must have all the elements

TEST(Concurrent, SPMC) {

    lock_fine_queue<int> q;
    std::vector<std::thread> threads;
    int concurrency_level = 4;
    int n = 999;
    threads.emplace_back( [&](){
        for (int i = 0; i < n; ++i) {
            q.push(i);
        }
    });

    std::vector<std::atomic<bool>> values(n);
    for (int i = 0; i < 3; ++i) {
        threads.emplace_back([&]() {
            for (int j = 0; j < n / 3; ++j) {
                int val;
                q.wait_and_pop(val);
                values[val].store(true, std::memory_order_relaxed);
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

// 7. Multiple Producers, Single Consumer
//    -> in the end we have all the elements that we pushed

TEST(Concurrent, MPSC) {

    lock_fine_queue<int> q;
    std::vector<std::thread> threads;
    int concurrency_level = 4;
    int n = 999;
    for (int i = 0; i < 3; ++i) {
        threads.emplace_back([i, &q, n]() {
            int beg = i * (n / 3);
            int end = (i + 1) * (n / 3);
            for (int j = beg; j < end; ++j) {
                q.push(j);
            }
        });
    }

    std::vector<std::atomic<bool>> values(n);
    threads.emplace_back([&]() {
        for (int j = 0; j < n; ++j) {
            int val;
            q.wait_and_pop(val);
            values[val].store(true, std::memory_order_relaxed);
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
TEST(Concurrent, MPMC) {

    lock_fine_queue<int> q;
    std::vector<std::thread> threads;
    int concurrency_level = 8;
    int n = 1200;
    for (int i = 0; i < 4; ++i) {
        threads.emplace_back([i, &q, n]() {
            int beg = i * (n / 4);
            int end = (i + 1) * (n / 4);
            for (int j = beg; j < end; ++j) {
                q.push(j);
            }
        });
    }

    std::vector<std::atomic<bool>> values(n);
    for (int i = 0; i < 4; ++i) {
        threads.emplace_back([n, &q, &values]() {
            for (int j = 0; j < n / 4; ++j) {
                int val;
                q.wait_and_pop(val);
                values[val].store(true, std::memory_order_relaxed);
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

// 9. Large number of push and pop operations
//    -> increase number of threas beyond typical
//    -> use very high N (1 000 000)
TEST(Stress, HighMPMC) {

    lock_fine_queue<int> q;
    std::vector<std::thread> threads;
    int number_of_producers = 50;
    int number_of_consumers = 50;
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
                q.wait_and_pop(val);
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
TEST(Stress, RandMPMC) {

    lock_fine_queue<int> q;
    std::vector<std::thread> threads;
    int number_of_producers = 20;
    int number_of_consumers = 20;
    int n = 50'000;

    for (int i = 0; i < number_of_producers; ++i) {
        threads.emplace_back([i, &q, n, number_of_producers]() {
            std::mt19937 gen(std::random_device{}());
            std::uniform_int_distribution<int> dist(0, 10);
            int beg = i * (n / number_of_producers);
            int end = (i + 1) * (n / number_of_producers);
            for (int j = beg; j < end; ++j) {
                q.push(j);
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
                int val;
                q.wait_and_pop(val);
                values[val].store(true, std::memory_order_relaxed);
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

    lock_fine_queue<ExeptInt> q;
    std::vector<std::thread> threads;
    int concurrency_level = 8;
    int n = 1200;
    for (int i = 0; i < 4; ++i) {
        threads.emplace_back([i, &q, n]() {
            
            std::mt19937 gen(std::random_device{}());
            std::uniform_int_distribution<int> dist(1, 6);
            int beg = i * (n / 4);
            int end = (i + 1) * (n / 4);
            for (int j = beg; j < end; ++j) {
                ExeptInt num(j, dist(gen) / 6);
                try {
                    q.push(num);
                } catch (const std::exception& e) {
                    ExeptInt num2(j, false);
                    q.push(num2);
                }
            }
        });
    }

    std::vector<std::atomic<bool>> values(n);
    for (int i = 0; i < 4; ++i) {
        threads.emplace_back([n, &q, &values]() {
            std::mt19937 gen(std::random_device{}());
            std::uniform_int_distribution<int> dist(1, 6);
            for (int j = 0; j < n / 4; ++j) {
                ExeptInt val(0, dist(gen) / 6);
                try {
                    q.wait_and_pop(val);
                    values[val.i_].store(true, std::memory_order_relaxed);
                } catch (const std::exception& e) {
                    ExeptInt val2(0, 0);
                    q.wait_and_pop(val2);
                    values[val2.i_].store(true, std::memory_order_relaxed);
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

/*
###################################################

            SHARED_PTR Functionality

###################################################
*/

// 12. Single Producer, Single Consumer
//    -> in the end we must have all the elemens in
//          the same order as we pushed
TEST(Concurrent, SPSC_PTR) {

    lock_fine_queue<int> q;
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
            values[i] = *q.wait_and_pop();
        }
    });

    for (int i = 0; i < concurrency_level; ++i) {
        threads[i].join();
    }

    for (int i = 0; i < n; ++i) {
        EXPECT_EQ(i, values[i]);
    }
}

// 13. Single Producer, Multiple Consumers
//    -> in the end we must have all the elements

TEST(Concurrent, SPMC_PTR) {

    lock_fine_queue<int> q;
    std::vector<std::thread> threads;
    int concurrency_level = 4;
    int n = 999;
    threads.emplace_back( [&](){
        for (int i = 0; i < n; ++i) {
            q.push(i);
        }
    });

    std::vector<std::atomic<bool>> values(n);
    for (int i = 0; i < 3; ++i) {
        threads.emplace_back([&]() {
            for (int j = 0; j < n / 3; ++j) {
                int val;
                val = *q.wait_and_pop();
                values[val].store(true, std::memory_order_relaxed);
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

// 14. Multiple Producers, Single Consumer
//    -> in the end we have all the elements that we pushed

TEST(Concurrent, MPSC_PTR) {

    lock_fine_queue<int> q;
    std::vector<std::thread> threads;
    int concurrency_level = 4;
    int n = 999;
    for (int i = 0; i < 3; ++i) {
        threads.emplace_back([i, &q, n]() {
            int beg = i * (n / 3);
            int end = (i + 1) * (n / 3);
            for (int j = beg; j < end; ++j) {
                q.push(j);
            }
        });
    }

    std::vector<std::atomic<bool>> values(n);
    threads.emplace_back([&]() {
        for (int j = 0; j < n; ++j) {
            int val;
            val = *q.wait_and_pop();
            values[val].store(true, std::memory_order_relaxed);
        }
    });

    for (int i = 0; i < concurrency_level; ++i) {
        threads[i].join();
    }

    for (int i = 0; i < n; ++i) {
        EXPECT_TRUE(values[i].load(std::memory_order_relaxed)) << "i= " << i << "\n";
    }
}

// 15. Multiple Producres, Multiple Consumers
//    -> in the end we have all the elements that we pushed
TEST(Concurrent, MPMC_PTR) {

    lock_fine_queue<int> q;
    std::vector<std::thread> threads;
    int concurrency_level = 8;
    int n = 1200;
    for (int i = 0; i < 4; ++i) {
        threads.emplace_back([i, &q, n]() {
            int beg = i * (n / 4);
            int end = (i + 1) * (n / 4);
            for (int j = beg; j < end; ++j) {
                q.push(j);
            }
        });
    }

    std::vector<std::atomic<bool>> values(n);
    for (int i = 0; i < 4; ++i) {
        threads.emplace_back([n, &q, &values]() {
            for (int j = 0; j < n / 4; ++j) {
                int val;
                val = *q.wait_and_pop();
                values[val].store(true, std::memory_order_relaxed);
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

// 16. Large number of push and pop operations
//    -> increase number of threas beyond typical
//    -> use very high N (1 000 000)
TEST(Stress, HighMPMC_PTR) {

    lock_fine_queue<int> q;
    std::vector<std::thread> threads;
    int number_of_producers = 50;
    int number_of_consumers = 50;
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
                val = *q.wait_and_pop();
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

// 17. Random delays
//     -> Introduce random sleep interval in producer 
//          and consumer threads
TEST(Stress, RandMPMC_PTR) {

    lock_fine_queue<int> q;
    std::vector<std::thread> threads;
    int number_of_producers = 20;
    int number_of_consumers = 20;
    int n = 50'000;

    for (int i = 0; i < number_of_producers; ++i) {
        threads.emplace_back([i, &q, n, number_of_producers]() {
            std::mt19937 gen(std::random_device{}());
            std::uniform_int_distribution<int> dist(0, 10);
            int beg = i * (n / number_of_producers);
            int end = (i + 1) * (n / number_of_producers);
            for (int j = beg; j < end; ++j) {
                q.push(j);
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
                int val;
                val = *q.wait_and_pop();
                values[val].store(true, std::memory_order_relaxed);
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

// 18. Exception handelling
//    -> Create a type, which in copy/move assignment/operator
//      throws exeptions with probability 1/6
//    -> try common tests with push pop to ensure that everything works

TEST(Exception, MPMC_PTR) {

    lock_fine_queue<ExeptInt> q;
    std::vector<std::thread> threads;
    int concurrency_level = 8;
    int n = 1200;
    for (int i = 0; i < 4; ++i) {
        threads.emplace_back([i, &q, n]() {
            
            std::mt19937 gen(std::random_device{}());
            std::uniform_int_distribution<int> dist(1, 6);
            int beg = i * (n / 4);
            int end = (i + 1) * (n / 4);
            for (int j = beg; j < end; ++j) {
                ExeptInt num(j, dist(gen) / 6);
                try {
                    q.push(num);
                } catch (const std::exception& e) {
                    ExeptInt num2(j, false);
                    q.push(num2);
                }
            }
        });
    }

    std::vector<std::atomic<bool>> values(n);
    for (int i = 0; i < 4; ++i) {
        threads.emplace_back([n, &q, &values]() {
            std::mt19937 gen(std::random_device{}());
            std::uniform_int_distribution<int> dist(1, 6);
            for (int j = 0; j < n / 4; ++j) {
                try {
                    auto val = q.wait_and_pop();
                    values[val->i_].store(true, std::memory_order_relaxed);
                } catch (const std::exception& e) {
                    auto val2 = q.wait_and_pop();
                    values[val2->i_].store(true, std::memory_order_relaxed);
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

TEST(FAILING_BENCH, push_pop_lock_fine_queue) {

    lock_fine_queue<int> q;
    std::vector<std::thread> threads;
    int kNumItems = 1000;
    int concurrency_level = 16;
    for (int i = 0; i < concurrency_level; ++i) {
        threads.emplace_back([i, kNumItems, &q]() {
            bool pusher = i < (16 / 2);
            if (pusher) {
                for (int i = 0; i < kNumItems; ++i) {
                    // std::cout << "We are going to push\n";
                    q.push(i);
                    // std::cout << "We pushed\n";
                }
            } else {
                for (int i = 0; i < kNumItems; ++i) {
                    // std::cout << "We are going to pop\n";
                    q.wait_and_pop();
                    // std::cout << "We poped\n";
                }    
            }
        });
    }
    for (int i = 0; i < concurrency_level; ++i) {
        threads[i].join();
    }
}