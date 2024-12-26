#pragma once

#include <queue>
#include <mutex>
#include <condition_variable>
#include <memory>

template <class T>
class thread_safe_std_queue {

    public:

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

    void try_pop(T& val);

    // Same as before but returns element in ponter

    std::shared_ptr<T> try_pop();

    // Checks that the queue is empty

    bool empty();


    private:

    std::queue<std::shared_ptr<T>> data_;
    std::mutex mutable mt_;
    std::condition_variable cv_;
};

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