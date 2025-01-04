#include <mutex>
#include <condition_variable>
#include <memory>

/*
Instead of std queue, which was the only protected data item
we shall use two members: 
    -head unique ptr
    -tail ptr

it will use classical node structure:

struct node {
    std::shared_ptr<T> data;
    std::unique_ptr<node> next;
};

    tail will always point to a dummy node
    head will always point to the first node on the queue

    When we are willing to delete node, we shall
    check, wheather head is equal to tail
    And if not, then change the pointer of head to the next one

    When we are willign to push a node
    we will allocate new value in std::shared_ptr
    then allocate new dummy node
    put the values into old dummy node (tail)
    and change tail to the new one

*/

template <class T>
class lock_fine_queue {

    struct Node {
        std::shared_ptr<T>    data_;
        std::unique_ptr<Node> next_;
    };

    Node* get_tail();

    std::unique_ptr<Node> pop_head();

    std::unique_lock<std::mutex> wait_for_data();

    std::unique_ptr<Node> wait_pop_head();

    std::unique_ptr<Node> wait_pop_head(T&);

    std::unique_ptr<Node> try_pop_head(T& val);

    std::unique_ptr<Node> try_pop_head();

    std::unique_ptr<Node> head_;
    Node*                 tail_;
    std::mutex mutable    mt_head_;
    std::mutex mutable    mt_tail_;
    std::condition_variable cv_;

    public:

    lock_fine_queue()
    : head_(new Node)
    , tail_(head_.get())
    {}

    lock_fine_queue(const lock_fine_queue&) = delete;

    lock_fine_queue& operator=(const lock_fine_queue&) = delete;

    void push(T val);

    void wait_and_pop(T& val);

    std::shared_ptr<T> wait_and_pop();

    bool try_pop(T& val);

    std::shared_ptr<T> try_pop();

    bool empty();

};

template<class T>
typename lock_fine_queue<T>::Node* 
lock_fine_queue<T>::get_tail() {

    std::lock_guard lg(mt_tail_);
    return tail_;
}

template<class T>
std::unique_ptr<typename lock_fine_queue<T>::Node> 
lock_fine_queue<T>::pop_head() {
    std::unique_ptr<Node> old_head = std::move(head_);
    head_ = std::move(old_head->next_);
    return old_head;
}

template<class T>
std::unique_lock<std::mutex> 
lock_fine_queue<T>::wait_for_data() {

    std::unique_lock<std::mutex> head_lock(mt_head_);
    cv_.wait(head_lock, [&]{return head_.get() != get_tail();});
    return std::move(head_lock);
}

template<class T>
void lock_fine_queue<T>::push(T val) {

    // 1. Create new data
    std::shared_ptr<T> data_new = std::make_shared<T>(std::move(val));
    // 2. Allocate new dummy node
    std::unique_ptr<Node> dummy(new Node);
    // 3. Get variable to store new tail
    // 4. Lock to do modifications
    {
        std::lock_guard<std::mutex> lg(mt_tail_);
        // 5. Copy the data
        tail_->data_ = data_new;
        Node* tail_new = dummy.get();
        // 6. Move dummy node as the next tail
        tail_->next_ = std::move(dummy);
        tail_ = tail_new;
    }
    cv_.notify_one();
}

template<class T>
std::unique_ptr<typename lock_fine_queue<T>::Node> 
lock_fine_queue<T>::try_pop_head() {
    
    // 1. Get the lock
    std::lock_guard<std::mutex> lg(mt_head_);
    // 2. Compare with the tail in case the queue is empty
    if (head_.get() == get_tail()) {
        return std::unique_ptr<Node>();
    } 
    return pop_head();
}

template<class T>
std::shared_ptr<T> lock_fine_queue<T>::try_pop() {

    std::unique_ptr<Node> old_head = try_pop_head();
    return old_head ? old_head->data_ : std::shared_ptr<T>();
}

template<class T>
std::unique_ptr<typename lock_fine_queue<T>::Node>
lock_fine_queue<T>::try_pop_head(T& val) {

    // 1. Get the lock
    std::lock_guard<std::mutex> lg(mt_head_);
    // 2. Compare with the tail in case the queue is empty
    if (head_.get() == get_tail()) {
        return std::unique_ptr<Node>();
    }
    val = std::move(*head_->data_);
    return pop_head(); 
}

template<class T>
bool lock_fine_queue<T>::try_pop(T& val) {

    std::unique_ptr<Node> old_head = try_pop_head(val);
    return old_head? true : false;
}

template<class T>
std::unique_ptr<typename lock_fine_queue<T>::Node> 
lock_fine_queue<T>::wait_pop_head(T& val) {

    std::unique_lock<std::mutex> head_lock(wait_for_data());
    val = std::move(*head_->data_);
    return pop_head();
}

template<class T>
void lock_fine_queue<T>::wait_and_pop(T& val) {

    // 1. Fetch old head
    std::unique_ptr<Node> old_head = wait_pop_head(val);
}

template<class T>
std::unique_ptr<typename lock_fine_queue<T>::Node> 
lock_fine_queue<T>::wait_pop_head() {

    std::unique_lock<std::mutex> head_lock(wait_for_data());
    return pop_head();
}


template<class T>
std::shared_ptr<T> lock_fine_queue<T>::wait_and_pop() {

    // 1. Fetch old head
    std::unique_ptr<Node> old_head = wait_pop_head();
    return old_head->data_;
}


template<class T>
bool lock_fine_queue<T>::empty() {

    std::lock_guard<std::mutex> lg(mt_head_);
    return head_.get() == get_tail();
}