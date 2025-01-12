#include <benchmark/benchmark.h>
#include "../include/lock-std-queue.hpp"

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
        if (state.thread_index() == 0) {
            while(!q.empty()) {
                q.try_pop();
            }
        }
    }

  lock_std_queue<int> q;
  static constexpr int kNumItems = 100000;
};

BENCHMARK_DEFINE_F(QueueFix, bench_push)(benchmark::State& state) {
    for (auto _ : state) {
        q.push(1);
    }
}

BENCHMARK_DEFINE_F(QueueFix, bench_pop)(benchmark::State& state) {
    int val;
    for (auto _ : state) {
        q.try_pop(val);
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
                while(!q.try_pop());
            }    
        }
    }
    state.SetItemsProcessed(state.iterations() * kNumItems);
}

BENCHMARK_DEFINE_F(QueueFix, bench_mpmc)(benchmark::State& state) {

    bool pusher = state.thread_index() % 2;

    for (auto _ : state) {
        if (pusher) {
            for (int i = 0; i < kNumItems; ++i) {
                q.push(i);
            }
        } else {
            for (int i = 0; i < kNumItems; ++i) {
                while(!q.try_pop());
            }    
        }
    }
    state.SetItemsProcessed(state.iterations() * kNumItems);
}

// Here write the amounts of threads that you want to use
// 2, 4, 8, 16
// Also you might want to use RealTime()
BENCHMARK_REGISTER_F(QueueFix, bench_push)
    ->Name("Push")
    ->UseRealTime()
    ->ThreadRange(1, 32);

BENCHMARK_REGISTER_F(QueueFix, bench_pop)
    ->Name("Pop")
    ->UseRealTime()
    ->ThreadRange(1, 32);

BENCHMARK_REGISTER_F(QueueFix, bench_spmc)
    ->Name("SPMC")
    ->UseRealTime()
    ->Unit(benchmark::kMicrosecond)
    ->ThreadRange(2, 32);

BENCHMARK_REGISTER_F(QueueFix, bench_mpmc)
    ->Name("MPMC")
    ->UseRealTime()
    ->Unit(benchmark::kMicrosecond)
    ->ThreadRange(2, 32);
BENCHMARK_MAIN();
