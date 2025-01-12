#pragma once

#include <memory>
#include <atomic>

template <class T>
class lock_free_spsc_queue {

private:

    struct Node {

        Node* next_;
        std::shared_ptr<T> data_;
    };

    Node* pop_head();

    Node* pop_head(T&);

    std::atomic<Node*> head_;
    std::atomic<Node*> tail_;

public:

    lock_free_spsc_queue()
    : head_(new Node())
    , tail_(head_.load())
    {}

    lock_free_spsc_queue(const lock_free_spsc_queue&) = delete;
    lock_free_spsc_queue& operator = (const lock_free_spsc_queue&) = delete;

    ~lock_free_spsc_queue() {
        while(pop()); 
        delete head_.load(std::memory_order_seq_cst);
    }

    // 1. push is lock-free, however is not supposed
    // for usage of more than 1 thread

    void push(T val);

    // 2. pop -- two versions: with std::shared and not

    std::shared_ptr<T> pop();

    bool pop(T& val);

    // 3. empty
    bool empty();

};

template<class T> 
void lock_free_spsc_queue<T>::push(T val) {

    // 1. create new data pointer 
    std::shared_ptr<T> data = std::make_shared<T>(std::move(val));
    // 2. create new dummy ptr
    Node* ptr = new Node();
    // 3. Save old_tail in a variable
    Node* old_tail = tail_.load(std::memory_order_acquire);
    // 4. Swap the data
    old_tail->data_.swap(data);
    // 5. Set next of the old tail to the ptr
    old_tail->next_ = ptr;
    // 6. store ptr in tail
    tail_.store(ptr, std::memory_order_release);
}

template<class T>
typename lock_free_spsc_queue<T>::Node* 
lock_free_spsc_queue<T>::pop_head() {

    Node* old_head = head_.load(std::memory_order_acquire);
    if (old_head == tail_.load(std::memory_order_acquire)) {
        return nullptr;
    }
    // 3. We can set the head to the 
    // old_head->next and return ptr to it
    head_.store(old_head->next_, std::memory_order_release);
    return old_head;
}

template<class T>
typename lock_free_spsc_queue<T>::Node* 
lock_free_spsc_queue<T>::pop_head(T& val) {

    Node* old_head = head_.load(std::memory_order_acquire);
    if (old_head == tail_.load(std::memory_order_acquire)) {
        return nullptr;
    }
    // Difference from the previos pop_head 
    // is that we use move here before we return and do any changes
    val = std::move(*old_head->data_);
    head_.store(old_head->next_, std::memory_order_release);
    return old_head;
}

template<class T> 
std::shared_ptr<T> lock_free_spsc_queue<T>::pop() {
    // 1. load old_head to work with
    Node* old_head = pop_head();
    // 2. Check that old_had is not null
    if (!old_head) {
        return std::shared_ptr<T>();
    }
    std::shared_ptr<T> res(old_head->data_);
    delete old_head;
    return res;
}

template<class T> 
bool lock_free_spsc_queue<T>::pop(T& val) {
    // 1. load old_head to work with
    Node* old_head = pop_head(val);
    // 2. Check that old_had is not null
    if (!old_head) {
        return false;
    }
    delete old_head;
    return true;
}

template<class T> 
bool lock_free_spsc_queue<T>::empty() {
    if (head_.load(std::memory_order_acquire) == tail_.load(std::memory_order_acquire)) {
        return true;
    }
    return false;
}



