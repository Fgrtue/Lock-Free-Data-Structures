#include <benchmark/benchmark.h>
#include "../include/lock-fine-queue.hpp"
#include <iostream>
#include <chrono>
#include <thread>

class QueueFix : public benchmark::Fixture {
    
public:

    void SetUp(::benchmark::State& state) override 
    {
        if (state.thread_index() == 0) {
            for (int i = 0; i < kNumItems; ++i) {
                q.push(1);
            }
        }
    } 

    void TearDown(::benchmark::State& state) override
    {
        if (state.thread_index() == 0) {
            while(!q.empty()) {
                q.try_pop();
            }
        }
    }

  lock_fine_queue<int> q;
  static constexpr int kNumItems = 1000;
};

BENCHMARK_DEFINE_F(QueueFix, bench_push_lock_fine_queue)(benchmark::State& state) {
    for (auto _ : state) {
        q.push(1);
    }
}

BENCHMARK_DEFINE_F(QueueFix, bench_pop_lock_fine_queue)(benchmark::State& state) {
    int val;
    for (auto _ : state) {
        q.try_pop(val);
    }
}

BENCHMARK_DEFINE_F(QueueFix, bench_push_pop_lock_fine_queue)(benchmark::State& state) {

    bool pusher     = state.thread_index() % 2;

    for (auto _ : state) {
        if (pusher) {
            for (int i = 0; i < kNumItems; ++i) {
                q.push(i);
            }
        } else {
            int val;
            for (int i = 0; i < kNumItems; ++i) {
                q.wait_and_pop(val);
                benchmark::DoNotOptimize(val);
            }    
        }
    }
    state.SetItemsProcessed(state.iterations() * kNumItems);
}

// Here write the amounts of threads that you want to use
// 2, 4, 8, 16
BENCHMARK_REGISTER_F(QueueFix, bench_push_lock_fine_queue)
    ->Name("PushLockQueue")
    ->UseRealTime()
    ->ThreadRange(1, 16);

BENCHMARK_REGISTER_F(QueueFix, bench_pop_lock_fine_queue)
    ->Name("PopLockQueue")
    ->UseRealTime()
    ->ThreadRange(1, 16);

BENCHMARK_REGISTER_F(QueueFix, bench_push_pop_lock_fine_queue)
    ->Name("PushPopLockQueue")
    ->UseRealTime()
    ->Unit(benchmark::kMicrosecond)
    ->ThreadRange(2, 16);
BENCHMARK_MAIN();