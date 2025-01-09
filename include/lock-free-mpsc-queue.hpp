#include <memory>
#include <atomic>
#include <iostream>
#include <assert.h>

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
class lock_free_mpsc_queue {

private:

    struct Node;

    struct external_count {
        int external_count_;
        Node* node_;
    };

    struct internal_count {
        unsigned internal_count_ : 30;
        unsigned ext_counters_    : 2;
    };

    struct Node {

        Node()
        {
            external_count ext_cnt;
            ext_cnt.external_count_ = 0;
            ext_cnt.node_ = nullptr;
            next_.store(ext_cnt);
            internal_count intr_cnt;
            intr_cnt.internal_count_  = 0; 
            intr_cnt.ext_counters_    = 2;
            counter_.store(intr_cnt);
            data_.store(nullptr);
        }

        void ref_release();

        std::atomic<external_count> next_;
        // # 1. This part should be changed for 
        // struct with counter and counter for external counts
        std::atomic<internal_count> counter_;
        // # 2. Becomes atomic pointer
        // In order to swap it in push
        std::atomic<T*> data_;
    };

    void increase_external(std::atomic<external_count>&, external_count&);

    void free_external(external_count& old_count);

    std::atomic<external_count> head_;
    std::atomic<external_count> tail_;

public:

    lock_free_mpsc_queue()
    {
        Node* node = new Node();
        external_count cnt;
        cnt.node_ = node;
        cnt.external_count_ = 2;
        tail_.store(cnt);
        head_.store(cnt);
    }

    lock_free_mpsc_queue(const lock_free_mpsc_queue&) = delete;
    lock_free_mpsc_queue& operator = (const lock_free_mpsc_queue&) = delete;

    ~lock_free_mpsc_queue() {

        while(pop());
        assert(empty());

        delete head_.load().node_;
    }

    void push(T val);

    // Sorrowly only one version of pop
    // is available in this case due to exception
    // safety. If we were to use pop(T&) version, then
    // at some point we have to copy or move T type, leading
    // to possibility of exception in copy/move assignment
    std::unique_ptr<T> pop();

    bool empty();

};

template<class T>
void lock_free_mpsc_queue<T>::increase_external(std::atomic<external_count>& target,
                                                external_count& old_count) {

    external_count count_new;
    do {
        count_new = old_count;
        ++count_new.external_count_;
    } while (!target.compare_exchange_strong(old_count, count_new));
    old_count.external_count_ = count_new.external_count_;
}

template<class T>
void lock_free_mpsc_queue<T>::Node::ref_release() {

    // # 6. Now we cannot just substract, so we need
    // CAS loop to decrease internal count structure by 1
    internal_count count_old = counter_.load();
    internal_count count_new;
    do {
        count_new = count_old;
        --count_new.internal_count_;
    } while (!counter_.compare_exchange_strong(count_old, count_new));
    // # 7. In the end we check if internal counter is 0 and number
    // of external counters is 2
    if (!count_new.internal_count_ && !count_new.ext_counters_) {
        delete this;
    }
}

template<class T>
void lock_free_mpsc_queue<T>::free_external(external_count& extr) {

    Node* const ptr = extr.node_;
    int const internal_upd = extr.external_count_ - 2;
    // # 8. Save old counter
    // Decrease the number of external counters
    // And update the internal
    internal_count count_old = ptr->counter_.load();
    internal_count count_new;
    do {
        count_new = count_old;
        --count_new.ext_counters_;
        count_new.internal_count_ += internal_upd;
    } while (!ptr->counter_.compare_exchange_strong(count_old, count_new));

    // # 9. In case internal counter is 0 and the number of external counters is 0
    // delete pointer 
    if (!count_new.internal_count_ && !count_new.ext_counters_) {
        delete ptr;
    }
}

template<class T> 
void lock_free_mpsc_queue<T>::push(T val) {

    // # 3. Instead of putting pointer in shared_ptr
    // put it in unique_ptr
    std::unique_ptr<T> data_new(new T(val));
    external_count count_new;
    count_new.node_ = new Node();
    count_new.external_count_ = 1;
    external_count old_tail = tail_.load();
    // # 4. Here we swtich to a similar loop as in pop
    // In the CAS if statement set the pointer of node that
    // unique ptr is pointing to, and release the ptr in unique pointer
    for (;;) {

        increase_external(tail_, old_tail);
        T* old_data = nullptr;
        if (old_tail.node_->data_.compare_exchange_strong(old_data, data_new.get())) {
            old_tail.node_->next_.store(count_new);
            old_tail = tail_.exchange(count_new); 
            free_external(old_tail);
            data_new.release();
            break ;
        }
        old_tail.node_->ref_release();
    }
}

template<class T> 
std::unique_ptr<T> lock_free_mpsc_queue<T>::pop() {

    external_count old_head = head_.load();
    for(;;) {
        // # 5. Fix this function, making it use external count atomic (head_ or tail_)
        increase_external(head_, old_head);
        Node* const ptr = old_head.node_;
        if (ptr == tail_.load().node_) {
            ptr->ref_release();
            return std::unique_ptr<T>();
        }
        if (head_.compare_exchange_strong(old_head, ptr->next_)) {
            T* const res = ptr->data_.exchange(nullptr);
            free_external(old_head);
            return std::unique_ptr<T>(res);
        }
        ptr->ref_release();
    }
}


template<class T> 
bool lock_free_mpsc_queue<T>::empty() {

    external_count head_count = head_.load();
    external_count tail_count = tail_.load();
    for(;;) {
        increase_external(head_, head_count);
        increase_external(tail_, tail_count);
        if (head_count.node_ == tail_count.node_) {
            head_count.node_->ref_release();
            tail_count.node_->ref_release();
            return true;
        }
        head_count.node_->ref_release();
        tail_count.node_->ref_release();
    }
    return false;
}