#pragma once

#include "../include/hazard-pointers.hpp"

#include <atomic>
#include <memory>
#include <iostream>

template<class T>
class lock_free_stack {

private:

    struct Node
    {

        std::shared_ptr<T> data_;
        Node* next_;
    };
    
    
    std::atomic<Node*> head_;
    hazard_pointers<Node> hazard_ptrs_;

public:

    lock_free_stack() : head_(nullptr) {}
    lock_free_stack(const lock_free_stack& other) = delete;
    lock_free_stack& operator= (const lock_free_stack& other) = delete;

    ~lock_free_stack() {
        while(pop());
    }

    void push(T);

    std::shared_ptr<T> pop();

    bool empty();
};

template<class T>
void  lock_free_stack<T>::push(T val) {

    std::shared_ptr<T> data(new T(std::move(val)));
    Node* head_new = new Node();
    head_new->data_ = data;
    do {
        head_new->next_ = head_.load(std::memory_order_acquire);
    } while (!head_.compare_exchange_strong(head_new->next_, head_new, std::memory_order_acq_rel));
}

template<class T>
std::shared_ptr<T>  lock_free_stack<T>::pop() {

    typename  hazard_pointers<Node>::HP* hp = hazard_ptrs_.acquire_hazard();
    Node* old_head = head_.load(std::memory_order_acquire);
    do {
        hp->ptr_.store(old_head, std::memory_order_release);
        Node* tmp = head_.load(std::memory_order_acquire);
        if (tmp != old_head) {
            continue;  // Restart if head changes
        }
    } while(old_head && !head_.compare_exchange_strong(old_head, old_head->next_, std::memory_order_acq_rel));
    hazard_ptrs_.release_hazard(hp);

    std::shared_ptr<T> res;
    if (old_head) {
        res.swap(old_head->data_);
        old_head->data_ = nullptr;
        hazard_ptrs_.reclaim_later(old_head);
    }
    return res;
}

template<class T>
bool lock_free_stack<T>::empty() {

    if (head_.load(std::memory_order_acquire) == nullptr) {
        return true;
    }
    return false;
}