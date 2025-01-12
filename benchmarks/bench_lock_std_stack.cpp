#include <benchmark/benchmark.h>
#include "../include/lock-std-stack.hpp"

class StackFix : public benchmark::Fixture {
    
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
                q.pop();
            }
        }
    }

  lock_std_stack<int> q;
  static constexpr int kNumItems = 100000;
};

BENCHMARK_DEFINE_F(StackFix, bench_push)(benchmark::State& state) {
    for (auto _ : state) {
        q.push(1);
    }
}

BENCHMARK_DEFINE_F(StackFix, bench_pop)(benchmark::State& state) {
    int val;
    for (auto _ : state) {
        q.pop();
    }
}

BENCHMARK_DEFINE_F(StackFix, bench_mpmc)(benchmark::State& state) {

    bool pusher = state.thread_index() % 2;

    for (auto _ : state) {
        if (pusher) {
            for (int i = 0; i < kNumItems; ++i) {
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
// Also you might want to use RealTime()
BENCHMARK_REGISTER_F(StackFix, bench_push)
    ->Name("Push")
    ->UseRealTime()
    ->ThreadRange(1, 4);

BENCHMARK_REGISTER_F(StackFix, bench_pop)
    ->Name("Pop")
    ->UseRealTime()
    ->ThreadRange(1, 4);

BENCHMARK_REGISTER_F(StackFix, bench_mpmc)
    ->Name("MPMC")
    ->UseRealTime()
    ->Unit(benchmark::kMicrosecond)
    ->ThreadRange(2, 8);
BENCHMARK_MAIN();
