#include <benchmark/benchmark.h>
#include "../include/lock-free-spsc-queue.hpp"
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
    {}

  lock_free_spsc_queue<int> q;
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

BENCHMARK_DEFINE_F(QueueFix, bench_spsc)(benchmark::State& state) {

    bool pusher = state.thread_index() % 2;
    for (auto _ : state) {
        if (pusher) {
            for (int i = 0; i < kNumItems; ++i) {
                q.push(i);
            }
        } else {
            for (int i = 0; i < kNumItems; ++i) {
                q.pop();
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
    ->Threads(1);

BENCHMARK_REGISTER_F(QueueFix, bench_spsc)
    ->Name("SPSC")
    ->UseRealTime()
    ->Unit(benchmark::kMicrosecond)
    ->Threads(2);
BENCHMARK_MAIN();
