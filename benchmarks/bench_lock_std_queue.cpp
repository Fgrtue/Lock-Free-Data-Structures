#include <benchmark/benchmark.h>
#include "../include/lock-std-queue.hpp"

class QueueFix : public benchmark::Fixture {
    
public:

    void SetUp(::benchmark::State& state) override 
    {
        if (state.thread_index() == 0) {
            for (int i = 0; i < 36; ++i) {
                q.push(1);
            }
        }
    } 

    void TearDown(::benchmark::State& state) override
    {}

  lock_std_queue<int> q;
  static constexpr int kNumItems = 100;
};

BENCHMARK_DEFINE_F(QueueFix, bench_push_lock_std_queue)(benchmark::State& state) {
    for (auto _ : state) {
        q.push(1);
    }
}

BENCHMARK_DEFINE_F(QueueFix, bench_pop_lock_std_queue)(benchmark::State& state) {
    int val;
    for (auto _ : state) {
        q.try_pop(val);
    }
}

BENCHMARK_DEFINE_F(QueueFix, bench_push_pop_lock_std_queue)(benchmark::State& state) {

    int halfThreads = state.threads() / 2;
    bool pusher     = (state.thread_index() < halfThreads);

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
// Also you might want to use RealTime()
BENCHMARK_REGISTER_F(QueueFix, bench_push_lock_std_queue)
    ->Name("PushLockQueue")
    ->UseRealTime()
    ->ThreadRange(1, 16);

BENCHMARK_REGISTER_F(QueueFix, bench_pop_lock_std_queue)
    ->Name("PopLockQueue")
    ->UseRealTime()
    ->ThreadRange(1, 16);

BENCHMARK_REGISTER_F(QueueFix, bench_push_pop_lock_std_queue)
    ->Name("PushPopLockQueue")
    ->UseRealTime()
    ->Unit(benchmark::kMicrosecond)
    ->ThreadRange(2, 16);

BENCHMARK_MAIN();