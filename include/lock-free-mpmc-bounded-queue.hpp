#pragma once

#include <vector>
#include <atomic>
#include <memory>

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