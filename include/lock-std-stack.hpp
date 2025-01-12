#include <stack>
#include <mutex>
#include <memory>

template<class T>
class lock_std_stack {

private:

    std::stack<std::shared_ptr<T>> data_;
    std::mutex mt_;

public:

    void push(T);

    std::shared_ptr<T> pop();

    bool empty();
};

template<class T>
void lock_std_stack<T>::push(T val) {

    std::shared_ptr<T> data_new = std::make_shared<T>(std::move(val));
    std::lock_guard<std::mutex> lg(mt_);
    data_.push(data_new);
}

template<class T>
std::shared_ptr<T> lock_std_stack<T>::pop() {

    std::shared_ptr<T> res;
    std::lock_guard<std::mutex> lg(mt_);
    if (!data_.empty()) {
        res = data_.top();
        data_.pop();
    }
    return res;
}

template<class T>
bool lock_std_stack<T>::empty() {

    std::lock_guard<std::mutex> lg(mt_);
    if (data_.empty()) {
        return true;
    }
    return false;
}