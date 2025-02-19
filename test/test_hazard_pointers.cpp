#include "hazard-pointers.hpp"

#include <gtest/gtest.h>
#include <iostream>
#include <thread>
#include <vector>

TEST(Basic, Aquire) {

    hazard_pointers<int> hazard_ptrs;

    hazard_pointers<int>::HP* hp = hazard_ptrs.acquire_hazard();
    hp->ptr_.store(new int(6));

    int* ptr_int = hp->ptr_.load();

    hazard_ptrs.release_hazard(hp);
    delete ptr_int;
}

TEST(Basic, NoRestore) {

    hazard_pointers<int> hazard_ptrs;

    hazard_pointers<int>::HP* hp = hazard_ptrs.acquire_hazard();
    hp->ptr_.store(new int(6));

    int* ptr_int = hp->ptr_.load();

    hazard_ptrs.release_hazard(hp);
    if (hazard_ptrs.in_hazard(ptr_int)) {
        hazard_ptrs.reclaim_later(ptr_int);
    } else {
        delete ptr_int;
    }
}

TEST(Basic, Restore) {

    hazard_pointers<int> hazard_ptrs;

    hazard_pointers<int>::HP* hp = hazard_ptrs.acquire_hazard();
    hp->ptr_.store(new int(6));

    int* ptr_int = hp->ptr_.load();

    if (hazard_ptrs.in_hazard(ptr_int)) {
        hazard_ptrs.reclaim_later(ptr_int);
        hazard_ptrs.release_hazard(hp);
    } else {
        ASSERT_TRUE(false);
    }
    hazard_ptrs.delete_nodes_with_no_hazards();
}

// Create Hazard Pointers struct
// Create pool of atomic ptr to integers
// Let threads to read the integer
// Put integer in hazard pointer and then try to delete from the array
// Once it managed, it will set the integer to true
// And if there is still a hazard ptr pointing to it, it shall
// put it into reclaim later
// otherwise it will delete it straight away

void claim_integer(hazard_pointers<int>& hazard_ptrs, std::vector<std::atomic<int*>>& arr,
                    std::vector<std::atomic<bool>>& res, int n) 
{
    for (int i = 0; i < n; ++i) {
        hazard_pointers<int>::HP* hp = hazard_ptrs.acquire_hazard();
        int* cur_int;
        int* old_int; 
        do {
            do {
                cur_int = arr[i].load();
                hp->ptr_.store(cur_int);
                old_int = arr[i].load();
            } while (cur_int != old_int);
        } while(old_int && !arr[i].compare_exchange_strong(old_int, nullptr));
        // free HP
        hazard_ptrs.release_hazard(hp);
        if (old_int) {
            res[*old_int].store(true);
            if (hazard_ptrs.in_hazard(old_int)) {
                hazard_ptrs.reclaim_later(old_int);
            } else {
                delete old_int;
            }
            hazard_ptrs.delete_nodes_with_no_hazards();
        }
    }
}

TEST(Concurrent, RestoreTwoThreads) {

    
    hazard_pointers<int> hazard_ptrs;
    int n = 1'000;
    int concurrency_level = 2;
    std::vector<std::atomic<int*>> arr(n);
    std::vector<std::atomic<bool>> res(n);
    std::vector<std::thread> threads;

    for (int i = 0; i < n; ++i) {

        arr[i].store(new int(i));
        res[i].store(false);
    }

    for (int i = 0; i < concurrency_level; ++i) {

        threads.emplace_back(claim_integer, std::ref(hazard_ptrs), std::ref(arr), std::ref(res), n);

    }

    for (int i = 0; i < concurrency_level; ++i) {
        threads[i].join();
    }

    for (int i = 0 ; i < n; ++i) {
        EXPECT_TRUE(res[i].load()) << "i = " << i << "\n";
    }
}

TEST(Concurrent, RestoreEightThreads) {

    
    hazard_pointers<int> hazard_ptrs;
    int n = 80'000;
    int concurrency_level = 8;
    std::vector<std::atomic<int*>> arr(n);
    std::vector<std::atomic<bool>> res(n);
    std::vector<std::thread> threads;

    for (int i = 0; i < n; ++i) {

        arr[i].store(new int(i));
        res[i].store(false);
    }

    for (int i = 0; i < concurrency_level; ++i) {

        threads.emplace_back(claim_integer, std::ref(hazard_ptrs), std::ref(arr), std::ref(res), n);

    }

    for (int i = 0; i < concurrency_level; ++i) {
        threads[i].join();
    }

    for (int i = 0 ; i < n; ++i) {
        EXPECT_TRUE(res[i].load()) << "i = " << i << "\n";
    }
}