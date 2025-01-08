#include <memory>
#include <atomic>
#include <iostream>

/*

The implementation of the queue supporst the functionality for
1. Single producer
2. Multiple consumers

We use reference counting approach in order to deal
with dagling pointers problem
i.e. we shall not delete nodes, where the number of
external references is non 0

The members of the queue are:
    - atomic node* head
    - atomic node* tail

Each node consitst of
    - shared ptr to data
    - atomic internal counter
    - atomic struct for next
        -> with next ptr
        -> external counter

Push
    1. Create new external counter
        -> with count value equal to 1
        -> next value equal to the dummy node
    2. Set value in tail for new ptr
    3. If managed, then set another next to the tail
    4. Atomically change the tail to the new external count (the one that we created)

Pop
    1. Take the pointer to the current head
    2. Increase the external pointer by 1
    3. Try to pop (exhange head with head_next)
    4. If you managed
        --> save the result
        --> call separate function
        --> atomically change the value of internal counter to += external - 2
        --> In case we have that internal counter became 0 => delete ptr
        --> return shared ptr
    5. If you failed -> decrease the reference in internal counter

Observe that we need only one refernce counter
Since multiple threads can h

For pop
    Check that head is not equal to tail
    if it is not, then pop as usual
*/

template <class T>
class lock_free_spmc_queue {

private:

    struct Node;

    struct external_count {
        int external_count_;
        Node* node_;
    };

    struct Node {

        Node()
        {
            external_count cnt;
            cnt.external_count_ = 0;
            cnt.node_ = nullptr;
            next_.store(cnt);
            internal_count_.store(0);
            data_.store(nullptr);

        }

        void ref_release();

        std::atomic<external_count> next_;
        std::atomic<int> internal_count_;
        std::atomic<T*> data_;

    };

    void increase_external(external_count&);

    void free_external(external_count& old_count);

    std::atomic<external_count> head_;
    std::atomic<external_count> tail_;

public:

    lock_free_spmc_queue()
    {
        Node* node = new Node();
        external_count cnt;
        cnt.node_ = node;
        cnt.external_count_ = 2;
        tail_.store(cnt);
        head_.store(cnt);
    }

    lock_free_spmc_queue(const lock_free_spmc_queue&) = delete;
    lock_free_spmc_queue& operator = (const lock_free_spmc_queue&) = delete;

    void push(T val);

    // Sorrowly only one version of pop
    // is available in this case due to exception
    // safety. If we were to use pop(T&) version, then
    // at some point we have to copy or move T type, leading
    // to possibility of exception in copy/move assignment
    std::unique_ptr<T> pop();


    ~lock_free_spmc_queue() {
        while(head_.load().node_->next_.load().node_) {
            pop();
        }
        delete head_.load().node_;
    }

    bool empty();

    // This check helps to find out
    // if the key structure for lock free
    // algorithm is lock-free
    bool extern_is_lock_free() {
        std::atomic<external_count> ec;
        return ec.is_lock_free();
    }
};

template<class T> 
void lock_free_spmc_queue<T>::push(T val) {

    std::unique_ptr<T> data_new(new T(std::move(val)));
    Node* node_new = new Node();
    external_count count_new;
    count_new.external_count_ = 1;
    count_new.node_ = node_new;
    external_count old_tail = tail_.load();
    old_tail.node_->next_.store(count_new);
    old_tail.node_->data_.store(data_new.get());
    tail_.store(count_new);
    data_new.release();
}

template<class T>
void lock_free_spmc_queue<T>::increase_external(external_count& old_count) {

    external_count count_new;
    do {
        count_new = old_count;
        ++count_new.external_count_;
    } while (!head_.compare_exchange_strong(old_count, count_new));
    old_count.external_count_ = count_new.external_count_;
}

template<class T>
void lock_free_spmc_queue<T>::Node::ref_release() {

    if (internal_count_.fetch_sub(1) == 1) {
        delete this;
    }
}

template<class T>
void lock_free_spmc_queue<T>::free_external(external_count& old_count) {

    Node* ptr = old_count.node_;
    int const internal_upd = old_count.external_count_ - 2;
    if (ptr->internal_count_.fetch_add(internal_upd) == -internal_upd) {
        delete ptr;
    }
}

template<class T> 
std::unique_ptr<T> lock_free_spmc_queue<T>::pop() {

    external_count old_count = head_.load();
    for(;;) {
        increase_external(old_count);
        Node* const ptr = old_count.node_;
        if (ptr == tail_.load().node_) {
            ptr->ref_release();
            return std::unique_ptr<T>();
        }
        external_count next_in_list = ptr->next_.load();
        if (head_.compare_exchange_strong(old_count, next_in_list)) {
            T* const res = ptr->data_.exchange(nullptr);
            free_external(old_count);
            return std::unique_ptr<T>(res);
        }
        ptr->ref_release();
    }
}


template<class T> 
bool lock_free_spmc_queue<T>::empty() {

    external_count old_count = head_.load();
    for(;;) {
        increase_external(old_count);
        if (old_count.node_ == tail_.load().node_) {
            old_count.node_->ref_release();
            return true;
        }
        old_count.node_->ref_release();
    }
    return false;
}