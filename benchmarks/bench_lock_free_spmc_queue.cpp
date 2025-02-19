#include <benchmark/benchmark.h>
#include "lock-free-spmc-queue.hpp"
#include <iostream>
#include <thread>
#include <chrono>

class QueueFix : public benchmark::Fixture {
    
public:

    void SetUp(::benchmark::State& state) override 
    {
        if (state.thread_index() == 0) {
            for (int i = 0; i < (kNumItems * state.threads()); ++i) {
                q.push(1);
            }
        }
    } 

    void TearDown(::benchmark::State& state) override
    {
         (void)state;
    }

  lock_free_spmc_queue<int> q;
  static constexpr int kNumItems = 100000;
};

BENCHMARK_DEFINE_F(QueueFix, bench_push)(benchmark::State& state) {
    for (auto _ : state) {
        q.push(1);
    }
}

BENCHMARK_DEFINE_F(QueueFix, bench_pop)(benchmark::State& state) {
    for (auto _ : state) {
        q.pop();
    }
}

BENCHMARK_DEFINE_F(QueueFix, bench_spmc)(benchmark::State& state) {

    bool pusher = (state.thread_index() == 1);
    for (auto _ : state) {
        if (pusher) {
            for (int i = 0; i < kNumItems * state.threads(); ++i) {
                q.push(i);
            }
        } else {
            for (int i = 0; i < kNumItems; ++i) {
                while(!q.pop());
            }    
        }
    }
    state.SetItemsProcessed(state.iterations() * kNumItems);
}

// Here write the amounts of threads that you want to use
// 2, 4, 8, 16
BENCHMARK_REGISTER_F(QueueFix, bench_push)
    ->Name("Push")
    ->UseRealTime()
    ->Threads(1);

BENCHMARK_REGISTER_F(QueueFix, bench_pop)
    ->Name("Pop")
    ->UseRealTime()
    ->ThreadRange(1, 32);

BENCHMARK_REGISTER_F(QueueFix, bench_spmc)
    ->Name("SPMC")
    ->UseRealTime()
    ->Unit(benchmark::kMicrosecond)
    ->ThreadRange(2, 32);
BENCHMARK_MAIN();
