#include <memory>
#include <atomic>

/*

1. Suppose only one thread is allowed to do pop
2. And multiple threads can push

Then each push will be do its own allocation
Take the tail, check that the value in tail is null
If it is, then atomically change the data to the new pointer
And set the next of tail to the next
And set another tail

If it is not, then continue in loop

For pop
    Check that head is not equal to tail
    if it is not, then pop as usual
*/

template <class T>
class lock_free_mpsc_queue {

private:

    struct Node {

        Node* next_;
        std::atomic<T*> data_;

        Node() 
        : next_(nullptr)
        , data_(nullptr)
        {}
    };

    Node* pop_head();

    Node* pop_head(T&);

    std::atomic<Node*> head_;
    std::atomic<Node*> tail_;

public:

    lock_free_mpsc_queue()
    : head_(new Node())
    , tail_(head_.load())
    {}

    lock_free_mpsc_queue(const lock_free_mpsc_queue&) = delete;
    lock_free_mpsc_queue& operator = (const lock_free_mpsc_queue&) = delete;

    // 1. push is lock-free, however is not supposed
    // for usage of more than 1 thread

    void push(T val);

    // 2. pop -- two versions: with std::shared and not

    std::shared_ptr<T> pop();

    bool pop(T& val);

    // 3. empty
    bool empty();

};

/*
Then each push will be do its own allocation
Take the tail, check that the value in tail is null
If it is, then atomically change the data to the new pointer
And set the next of tail to the next
And set another tail

If it is not, then continue in loop
*/

template<class T> 
void lock_free_mpsc_queue<T>::push(T val) {

    // 1. create new node
    Node* ptr = new Node();
    // 2. create new pointer for value
    T* val_ptr = new T(std::move(val));
    for(;;) {
        T* data = nullptr;
        if (tail_.load()->data_.compare_exchange_strong(data, val_ptr)) {
            tail_.load()->next_ = ptr;
            tail_.exchange(ptr);
            break ;
        }
    }
}

// template<class T> 
// void lock_free_mpsc_queue<T>::push(T val) {

//     // 1. create new node
//     Node* ptr = new Node();
//     // 2. create new pointer for value
//     T* val_ptr = new T(std::move(val));
//     for(;;) {
//         T* data = nullptr;
//         Node* old_tail = tail_.load();
//         if (old_tail->data_.compare_exchange_strong(data, val_ptr)) {
//             old_tail->next_ = ptr;
//             tail_.store(ptr);
//             break ;
//         }
//     }
// }

template<class T>
typename lock_free_mpsc_queue<T>::Node* 
lock_free_mpsc_queue<T>::pop_head() {

    Node* old_head = head_.load();
    if (old_head == tail_.load()) {
        return nullptr;
    }
    // 3. We can set the head to the 
    // old_head->next and return ptr to it
    head_.store(old_head->next_);
    return old_head;
}

// template<class T>
// typename lock_free_mpsc_queue<T>::Node* 
// lock_free_mpsc_queue<T>::pop_head(T& val) {

//     Node* old_head = head.load();
//     if (old_head == tail.load()) {
//         return nullptr;
//     }
//     // Difference from the previos pop_head 
//     // is that we use move here before we return and do any changes
//     val = std::move(old_head->data_.load());
//     head.store(old_head->next_);
//     return old_head;
// }

template<class T> 
std::shared_ptr<T> lock_free_mpsc_queue<T>::pop() {
    // 1. load old_head to work with
    Node* old_head = pop_head();
    // 2. Check that old_had is not null
    if (!old_head) {
        return std::shared_ptr<T>();
    }
    std::shared_ptr<T> res(old_head->data_.load());
    delete old_head;
    return res;
}

// template<class T> 
// bool lock_free_mpsc_queue<T>::pop(T& val) {
//     // 1. load old_head to work with
//     Node* old_head = pop_head(val);
//     // 2. Check that old_had is not null
//     if (!old_head) {
//         return false;
//     }
//     delete old_head;
//     return true;
// }

template<class T> 
bool lock_free_mpsc_queue<T>::empty() {
    if (head_.load() == tail_.load()) {
        return true;
    }
    return false;
}



