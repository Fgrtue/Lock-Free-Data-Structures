#pragma once

#include <atomic>
#include <memory>
#include <assert.h>
#include <iostream>
#include <mutex>

/*
    Hazard pointers plan

    We have two methods:

    1. HPointers::Aquire

        1. Walk through the list of hazard pointers =>
        try to return the one that is free
        if no such => create a new one and push it in front of the
        list

    2. HPointers::Release
        - Just sets active and ptr to null

    3. Reclaimation functions

        1) Add to reclaim later
            -> simply add another node to the front of the list
        2) Delete nodes with no hazards
            -> exchange the node from the node list with null
            -> walk through the list
            -> delete nodes that we can delete
            -> save nodes that we cannot delete
*/

template<class N>
class hazard_pointers {

    public:

    hazard_pointers() 
    : hazards_list_(nullptr)
    , reclamation_list_(nullptr) 
    , recl_list_sz_(0) 
    {}
    
    hazard_pointers(const hazard_pointers& other) = delete;
    hazard_pointers& operator=(const hazard_pointers& other) = delete;

    ~hazard_pointers() {

        HP* hzrd_ptr = hazards_list_.load(std::memory_order_acquire);
        HP* hzrd_next_ptr;
        while (hzrd_ptr)
        {
            hzrd_next_ptr = hzrd_ptr->next_;
            assert(!hzrd_ptr->active_.load(std::memory_order_acquire));
            delete hzrd_ptr;
            hzrd_ptr = hzrd_next_ptr;
        }
        hazards_list_.store(nullptr, std::memory_order_release);
        delete_nodes_with_no_hazards();
    }

    struct HP {

        HP(): next_(nullptr), ptr_(nullptr), active_(false) {}

        HP*                 next_;
        std::atomic<N*>     ptr_;
        std::atomic<bool>   active_;
    };

    HP* acquire_hazard();

    void release_hazard(HP*);

    bool in_hazard(N*);

    struct node_recl {

        node_recl(N* data)
        : data_(data)
        , next_(nullptr)
        {} 

        void delete_node() {
            delete static_cast<N*>(data_);
            data_ = nullptr;
            next_ = nullptr;
        }

        N*   data_;
        node_recl*  next_;
    };

    void insert_reclaim(node_recl*);

    void reclaim_later(N* node);

    void delete_nodes_with_no_hazards();

    private:

    std::atomic<HP*>        hazards_list_;
    std::atomic<node_recl*> reclamation_list_;
    std::atomic<int>        recl_list_sz_;
    static constexpr int    max_recl_size_ = 20'000;
    std::mutex              mutex_scan_;
};

template<class N>
typename hazard_pointers<N>::HP*
hazard_pointers<N>::acquire_hazard() {

    HP* ptr = hazards_list_.load(std::memory_order_acquire);
    bool expect = false;
    for(; ptr ; ptr = ptr->next_) {

        if (ptr->active_.compare_exchange_strong(expect, true)) {
            return ptr;
        }
    }
    HP* hazard_new = new HP();
    hazard_new->active_.store(true, std::memory_order_release);
    do {
        hazard_new->next_ = hazards_list_.load(std::memory_order_acquire);
    } while (!hazards_list_.compare_exchange_strong(hazard_new->next_, hazard_new));
    return hazard_new;
}

template<class N>
void hazard_pointers<N>::release_hazard(HP* hp) {
    hp->ptr_.store(nullptr, std::memory_order_release);
    hp->active_.store(false, std::memory_order_release);
}

template<class N>
bool hazard_pointers<N>::in_hazard(N* data) {

    HP* cur = hazards_list_.load(std::memory_order_acquire);
    for (;cur; cur = cur->next_) {
        assert(cur);
        if (cur->ptr_.load(std::memory_order_acquire) == data) {
            return true;
        }
    }
    return false;
}


template<class N>
void hazard_pointers<N>::insert_reclaim(node_recl* reclaim_new) {

    reclaim_new->next_ = reclamation_list_.load(std::memory_order_acquire);
    while (!reclamation_list_.compare_exchange_strong(reclaim_new->next_, reclaim_new, std::memory_order_acq_rel));
    int sz;
    if ((sz = recl_list_sz_.fetch_add(1) >= max_recl_size_)) {
        delete_nodes_with_no_hazards();
    }
}

template<class N>
void hazard_pointers<N>::reclaim_later(N* node) {

    node_recl* reclaim_new = new node_recl(node);
    insert_reclaim(reclaim_new);
}

template<class N>
void hazard_pointers<N>::delete_nodes_with_no_hazards() {

    std::unique_lock<std::mutex> ul{mutex_scan_, std::defer_lock_t()};

    if (!ul.try_lock()) {
        return ;
    }
    int cur_sz;
    do {
        cur_sz = recl_list_sz_.load(std::memory_order_acquire);
    } while (!recl_list_sz_.compare_exchange_strong(cur_sz, 0, std::memory_order_acq_rel));

    node_recl* list_ptr = reclamation_list_.exchange(nullptr);
    node_recl* next_list;
    while (list_ptr) {
        next_list = list_ptr->next_;
        if (in_hazard(list_ptr->data_)) {
            insert_reclaim(list_ptr);
        } else {
            list_ptr->delete_node();
            delete list_ptr;
        }
        list_ptr = next_list;
    }
}

