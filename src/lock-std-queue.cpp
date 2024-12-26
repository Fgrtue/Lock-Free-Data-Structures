#include "../include/lock-std-queue.hpp"

template <class T>
thread_safe_std_queue::push(T new_value) {

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
thread_safe_std_queue::wait_and_pop(T& val) {

    std::unique_lock<std::mutex> lg(mt_);
    cv_.wait(lg, [this](){ return !data_.empty()});
    val = std::move(*data_.front());
    data_.pop();
}

template <class T>
std::shared_ptr<T> thread_safe_std_queue::wait_and_pop() {

    std::unique_lock<std::mutex> lg(mt_);
    cv_.wait(lg, [this](){ return !data_.empty()});
    auto p = data_.front();
    data_.pop();
    return p;
}

template <class T>
bool thread_safe_std_queue::try_pop(T& val) {

    std::lock_guard<std::mutex> lg(mt_);
    if (data_.empty()) {
        return false;
    }
    val = std::move(*data_.front());
    data_.pop();
    return true;
}

template <class T>
std::shared_ptr<T> thread_safe_std_queue::try_pop() {

    std::lock_guard<std::mutex> lg(mt_);
    if (data_.empty()) {
        return std::shared_ptr<T>();
    }
    auto ptr = data_.front();
    data_.pop();
    return ptr;
}

template <class T>
bool thread_safe_std_queue::empty() {
    std::lock_guard<std::mutex> lg(mt_);
    return data_.empty();
}