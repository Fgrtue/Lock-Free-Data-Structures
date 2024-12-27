#pragma once

#include <queue>
#include <mutex>
#include <condition_variable>
#include <memory>

template <class T>
class lock_std_queue {

    public:

    lock_std_queue() = default;
    lock_std_queue(const lock_std_queue&) = delete;
    lock_std_queue& operator=(const lock_std_queue&) = delete;

    // Pushes a value into the queue
    // Notifies oter threads that were waiting on pop
    void push(T new_value);

    // Wait in case queue is empty
    // Delete value from the queue once it is there 
    void wait_and_pop(T& val);

    // Same as previous but 
    // pay attention to construction of shared_ptr
    std::shared_ptr<T> wait_and_pop();

    // On empty queue it must return false.
    // Retrieve and remove the front element, moving it into value, 
    // returning true.
    bool try_pop(T& val);

    // Same as before but returns element in ponter
    std::shared_ptr<T> try_pop();

    // Checks that the queue is empty
    bool empty();

    private:

    std::queue<std::shared_ptr<T>> data_;
    std::mutex mutable mt_;
    std::condition_variable cv_;
};

template <class T>
void lock_std_queue<T>::push(T new_value) {

    // Observe that allocation is done outside of the queue
    // Therefore malloc is not called while holding a lock
    std::shared_ptr<T> p = std::make_shared<T>(std::move(new_value));
    std::lock_guard<std::mutex> lg_(mt_);
    data_.push(p);
    // An issue with this notify might be
    // If there are two threads, sleeping on wait_and_pop
    // and only one thread gets notified, but then is killed (exception)
    // then the other one shall sleep infinitely, even though it could
    // have poped an element
    cv_.notify_one();
}

template <class T>
void lock_std_queue<T>::wait_and_pop(T& val) {

    std::unique_lock<std::mutex> lg(mt_);
    cv_.wait(lg, [this](){ return !data_.empty();});
    val = std::move(*data_.front());
    data_.pop();
}

template <class T>
std::shared_ptr<T> lock_std_queue<T>::wait_and_pop() {

    std::unique_lock<std::mutex> lg(mt_);
    cv_.wait(lg, [this](){ return !data_.empty();});
    auto p = data_.front();
    data_.pop();
    return p;
}

template <class T>
bool lock_std_queue<T>::try_pop(T& val) {

    std::lock_guard<std::mutex> lg(mt_);
    if (data_.empty()) {
        return false;
    }
    val = std::move(*data_.front());
    data_.pop();
    return true;
}

template <class T>
std::shared_ptr<T> lock_std_queue<T>::try_pop() {

    std::lock_guard<std::mutex> lg(mt_);
    if (data_.empty()) {
        return std::shared_ptr<T>();
    }
    auto ptr = data_.front();
    data_.pop();
    return ptr;
}

template <class T>
bool lock_std_queue<T>::empty() {
    std::lock_guard<std::mutex> lg(mt_);
    return data_.empty();
}

/*
1. Implement the Constructor
Default Constructor:
Implement a default constructor for threadsafe_queue.
Delete Copy Operations:
Delete the copy constructor and copy assignment operator to prevent copying.

2. Implement the push Function
Function Declaration:
void push(T new_value).
    - Pushes a value into the queue
    - Notifies oter threads that were waiting on pop

3. Implement wait_and_pop (Reference Version)
Function Declaration:
void wait_and_pop(T& value).
   

4. Implement wait_and_pop (Shared Pointer Version)
Function Declaration:
std::shared_ptr<T> wait_and_pop().
Function Implementation:

5. Implement try_pop (Reference Version)
Function Declaration:
bool try_pop(T& value).
Function Implementation:
 
6. Implement try_pop (Shared Pointer Version)
Function Declaration:
Declare a public member function std::shared_ptr<T> try_pop().
Function Implementation:

7. Implement the empty Function
Function Declaration:
bool empty() const.
Function Implementation:

8. Enhance Exception Safety (Optional)
Replace notify_one with notify_all:
Modify the push function to use data_cond.notify_all() instead of notify_one() if desired.
Handle Exceptions in wait_and_pop:
Implement additional exception handling to ensure all waiting threads are properly notified in case of exceptions.

9. Optimize Performance (Optional)
Move Semantics:
Ensure that your queue efficiently handles move-only types by appropriately using std::move.
Fine-Grained Locking:
Explore more granular locking mechanisms to improve concurrency, if applicable.

10. Document Your Code
Add Comments:
Provide clear comments explaining the purpose of each part of your code.
Create Documentation:
Optionally, create separate documentation outlining the design decisions and usage of your threadsafe_queue.

11. Create a Test Program
Set Up main.cpp:
Create a main.cpp file to test your threadsafe_queue.
Include the Header:
Include your threadsafe_queue.h in main.cpp.
Instantiate the Queue:
Create an instance of threadsafe_queue<int>.

12. Implement Producer and Consumer Threads
Producer Thread:
Implement a thread that pushes integers into the queue.
Add delays to simulate work (e.g., using std::this_thread::sleep_for).
Consumer Thread:
Implement a thread that pops integers from the queue using wait_and_pop.
Print the consumed values to verify correctness.

Feel free to revisit each task, experiment with different approaches, and integrate additional features as you become more comfortable with concurrent programming concepts. Happy coding!

*/