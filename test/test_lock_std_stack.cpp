#include "lock-std-stack.hpp"

#include <gtest/gtest.h>
#include <iostream>
#include <thread>
#include <vector>
#include <random>
#include <chrono>
#include  <stdexcept>
#include <memory>

// 1. Empty
// 2. push pop
// 3. Concurrent push pop 2 threads
// 4. Concurrent push pop 16 threads
// 5. Stress push pop
// 6. Stress Rand Delays Push Pop
// 7. Exception Push Pop

TEST(Basic, Empty) {

    lock_std_stack<int> s;

    EXPECT_TRUE(s.empty());
}

TEST(Basic, Empty2) {

    lock_std_stack<int> s;

    s.push(1);

    EXPECT_FALSE(s.empty());
}

TEST(Basic, PushPop) {

    lock_std_stack<int> s;

    s.push(1);
    s.push(2);
    s.push(3);

    auto res = s.pop();
    ASSERT_TRUE(res);
    EXPECT_EQ(*res, 3);
    res = s.pop();    
    ASSERT_TRUE(res);
    EXPECT_EQ(*res, 2);    
    res = s.pop();    
    ASSERT_TRUE(res);
    EXPECT_EQ(*res, 1);
}

TEST(Concurrent, TwoTrheads) {

    lock_std_stack<int> s;
    int n = 1000;
    int number_of_producers = 1;
    int number_of_consumers = 1;
    std::vector<std::thread> threads;
    std::vector<std::atomic<bool>> values(n);

    for (int i = 0; i < n; ++i) {
        values[i].store(false);
    }

    for (int i = 0; i < number_of_producers; ++i) {
        threads.emplace_back([&]() {
            for(int j = 0; j < n; ++j) {
                s.push(j);
            }
        });
    }

    for (int i = 0; i < number_of_consumers; ++i) {
        threads.emplace_back([&]() {
            for(int j = 0; j < (n / number_of_consumers); ++j) {
                std::shared_ptr<int> res;
                while((res = s.pop()) == nullptr);
                values[*res].store(true);
            }
        });
    }

    for(int i = 0; i < number_of_producers + number_of_consumers; ++i) {
        threads[i].join();
    }

    for (int i = 0; i < n; ++i) {
        EXPECT_TRUE(values[i].load());
    }
}

TEST(Concurrent, MoreTrheads) {

    lock_std_stack<int> s;
    int n = 16'000;
    int number_of_producers = 4;
    int number_of_consumers = 4;
    std::vector<std::thread> threads;
    std::vector<std::atomic<bool>> values(n);

    for (int i = 0; i < n; ++i) {
        values[i].store(false);
    }

    for (int i = 0; i < number_of_producers; ++i) {
        threads.emplace_back([&s, number_of_producers, i, n]() {
            int beg = i * (n / number_of_producers);
            int end = (i + 1) * (n / number_of_producers);
            for(int j = beg; j < end; ++j) {
                s.push(j);
            }
        });
    }

    for (int i = 0; i < number_of_consumers; ++i) {
        threads.emplace_back([&]() {
            for(int j = 0; j < (n / number_of_consumers); ++j) {
                std::shared_ptr<int> res;
                while((res = s.pop()) == nullptr);
                values[*res].store(true);
            }
        });
    }

    for(int i = 0; i < number_of_producers + number_of_consumers; ++i) {
        threads[i].join();
    }

    for (int i = 0; i < n; ++i) {
        EXPECT_TRUE(values[i].load());
    }
}

TEST(Stress, HighTrheads) {

    lock_std_stack<int> s;
    int n = 6'000'000;
    int number_of_producers = 4;
    int number_of_consumers = 4;
    std::vector<std::thread> threads;
    std::vector<std::atomic<bool>> values(n);

    for (int i = 0; i < n; ++i) {
        values[i].store(false);
    }

    for (int i = 0; i < number_of_producers; ++i) {
        threads.emplace_back([&s, number_of_producers, i, n]() {
            int beg = i * (n / number_of_producers);
            int end = (i + 1) * (n / number_of_producers);
            for(int j = beg; j < end; ++j) {
                s.push(j);
            }
        });
    }

    for (int i = 0; i < number_of_consumers; ++i) {
        threads.emplace_back([&]() {
            for(int j = 0; j < (n / number_of_consumers); ++j) {
                std::shared_ptr<int> res;
                while((res = s.pop()) == nullptr);
                values[*res].store(true);
            }
        });
    }

    for(int i = 0; i < number_of_producers + number_of_consumers; ++i) {
        threads[i].join();
    }

    for (int i = 0; i < n; ++i) {
        EXPECT_TRUE(values[i].load());
    }
}

TEST(Stress, RandSleep) {

    lock_std_stack<int> s;
    int n = 50'000;
    int number_of_producers = 4;
    int number_of_consumers = 4;
    std::vector<std::thread> threads;
    std::vector<std::atomic<bool>> values(n);

    for (int i = 0; i < n; ++i) {
        values[i].store(false);
    }

    for (int i = 0; i < number_of_producers; ++i) {
        threads.emplace_back([&s, number_of_producers, i, n]() {
            std::mt19937 gen(std::random_device{}());
            std::uniform_int_distribution<int> dist(0, 10);
            int beg = i * (n / number_of_producers);
            int end = (i + 1) * (n / number_of_producers);
            for(int j = beg; j < end; ++j) {
                s.push(j);
                std::this_thread::sleep_for(std::chrono::milliseconds(dist(gen)));
            }
        });
    }

    for (int i = 0; i < number_of_consumers; ++i) {
        threads.emplace_back([&]() {
            for(int j = 0; j < (n / number_of_consumers); ++j) {
                std::mt19937 gen(std::random_device{}());
                std::uniform_int_distribution<int> dist(0, 10);
                std::shared_ptr<int> res;
                while((res = s.pop()) == nullptr);
                values[*res].store(true);
                std::this_thread::sleep_for(std::chrono::milliseconds(dist(gen)));
            }
        });
    }

    for(int i = 0; i < number_of_producers + number_of_consumers; ++i) {
        threads[i].join();
    }

    for (int i = 0; i < n; ++i) {
        EXPECT_TRUE(values[i].load());
    }
}

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

TEST(Stress, Exceptions) {

    lock_std_stack<ExeptInt> s;
    int n = 10'000;
    int number_of_producers = 4;
    int number_of_consumers = 4;
    std::vector<std::thread> threads;
    std::vector<std::atomic<bool>> values(n);

    for (int i = 0; i < n; ++i) {
        values[i].store(false);
    }

    for (int i = 0; i < number_of_producers; ++i) {
        threads.emplace_back([&s, number_of_producers, i, n]() {
            int beg = i * (n / number_of_producers);
            int end = (i + 1) * (n / number_of_producers);
            std::mt19937 gen(std::random_device{}());
            std::uniform_int_distribution<int> dist(1, 6);
            for(int j = beg; j < end; ++j) {
                ExeptInt num(j, dist(gen) / 6);
                try {
                    s.push(num);
                } catch (const std::exception& e) {
                    ExeptInt num2(j, false);
                    s.push(num2);
                }
            }
        });
    }

    for (int i = 0; i < number_of_consumers; ++i) {
        threads.emplace_back([&]() {
            for(int j = 0; j < (n / number_of_consumers); ++j) {
                std::shared_ptr<ExeptInt> res;
                while((res = s.pop()) == nullptr);
                values[res->i_].store(true, std::memory_order_relaxed);
            }
        });
    }

    for(int i = 0; i < number_of_producers + number_of_consumers; ++i) {
        threads[i].join();
    }

    for (int i = 0; i < n; ++i) {
        EXPECT_TRUE(values[i].load());
    }
}