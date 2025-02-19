#pragma once

#include <vector>
#include <atomic>
#include <memory>

/*

    The implementation supports multiple producer
    multiple consumer lock-free queue as follows 
    from the name. 
    
    We have three methods:
    1. push
    2. pop
    3. empty

    Let us first consider the members
    -> atomic head
    -> atomic tail
    -> array with nodes and atomic generation

    The implemenatation of push and pop I find a bit similar
    to ticket lock. I will explain that later
    
    Let us walk through the algorithm first
    
    PUSH

    In a while loop
    1. Get the old_head value
    2. Compute the value to which you want to update head
    3. Check that queue is not empty (by comparing with the tail)
    4. Get the GENERATION of the cell you are pointing at
        -> this is the key part, since only the thread which came
        -> in the right generation will be able to update the queue
        -> this will make sure that no thread that somehow luckily made
        -> a round turn (imagine pushes and pops were happenning very quickly)
        -> can update the cell in the queue
    5. If your cell is the same gen as your head is, try cas, 
        if fail        -> repeat the loop
        if succeeded   -> set the next generation as new_head (for the future popper)
    
    POP
    
    Pop does exactly the same in a while loop,
    the only difference is that we have to compare
    generation with old_tail + 1, and set the generation for
    the future pusher, which will be tail index + size of the queue
    -> i.e. a pusher has to do a round trip before it is able to push there

    Let us consider constructor for the queue. One of the
    optimization that we make is to use power of 2 in the size
    of the queue. This enables quick modulo operation by using &
    -> which will erase the most significant bit preserving the rest 
*/

template<class T>
class lock_free_mpmc_bounded_queue {

private:

    struct Node {
        std::atomic<int> gen_;
        T*                content_;

        Node() : gen_(0), content_(nullptr) {}
    };

    std::unique_ptr<Node[]> data_;
    std::atomic<int> head_;
    std::atomic<int> tail_;
    int              size_;
    int              MASK;

public: 

    lock_free_mpmc_bounded_queue()
    : lock_free_mpmc_bounded_queue(1e6)
    {}

    lock_free_mpmc_bounded_queue(int size) {

        size_ = 1;
        while(size_ < size) {
            size_ <<= 1;
        }
        data_ = std::make_unique<Node[]>(size_);
        for (int i = 0; i < size_; ++i) {
            data_[i].gen_.store(i, std::memory_order_release);
        }
        MASK = size_ - 1;
        head_.store(0, std::memory_order_release);
        tail_.store(0, std::memory_order_release);
    }

    lock_free_mpmc_bounded_queue(lock_free_mpmc_bounded_queue& other) = delete;
    lock_free_mpmc_bounded_queue& operator = (lock_free_mpmc_bounded_queue& other) = delete;

    ~lock_free_mpmc_bounded_queue() {

        while(pop());
    }

    bool push(T);

    std::unique_ptr<T> pop();

    bool empty();
};

template<class T>
bool lock_free_mpmc_bounded_queue<T>::push(T val) {

    std::unique_ptr<T> data_new(new T(std::move(val)));
    int old_head;
    int head_new;
    for (;;) {
        old_head = head_.load(std::memory_order_acquire);
        head_new = old_head + 1;
        if ((head_new & MASK) == (tail_.load(std::memory_order_acquire) & MASK)) {
            return false;
        }
        int node_gen = data_[old_head & MASK].gen_.load(std::memory_order_acquire);
        if (old_head != node_gen) {
            continue;
        }
        if (head_.compare_exchange_weak(old_head, head_new, std::memory_order_acq_rel)) {
            Node& cell = data_[old_head & MASK];
            cell.content_ = data_new.get();
            data_new.release();
            cell.gen_.store(head_new, std::memory_order_release);
            return true;
        }
    }
}

template<class T>
std::unique_ptr<T> lock_free_mpmc_bounded_queue<T>::pop() {

    int old_tail;
    int tail_new;
    for(;;) {
        old_tail = tail_.load(std::memory_order_acquire);
        tail_new = old_tail + 1;
        if ((old_tail & MASK) == (head_.load(std::memory_order_acquire) & MASK)) {
            return std::unique_ptr<T>();
        }
        int node_gen = data_[old_tail & MASK].gen_.load(std::memory_order_acquire);
        if (tail_new != node_gen) {
            continue;
        }
        if (tail_.compare_exchange_weak(old_tail, tail_new, std::memory_order_acq_rel)) {
            Node& cell = data_[old_tail & MASK];
            T* ptr = cell.content_;
            cell.content_ = nullptr;
            cell.gen_.store(old_tail + size_, std::memory_order_release);
            return std::unique_ptr<T>(ptr); 
        }
    }
}

template<class T>
bool lock_free_mpmc_bounded_queue<T>::empty() {
    
    if(head_.load(std::memory_order_acquire) == tail_.load(std::memory_order_acquire)) {
        return true;
    }
    return false;
}