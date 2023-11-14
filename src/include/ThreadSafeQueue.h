#include <iostream>
#include <mutex>
#include <queue>
#include <condition_variable>

template<typename T>
class ThreadSafeQueue {
public:
    void push(const T& data) {
        std::lock_guard<std::mutex> lock(lockMutex);
        lockQueue.emplace(data);
        lockCv.notify_one();
    }
    T pop() {
        std::unique_lock<std::mutex> lock(lockMutex);
        lockCv.wait(lock,[&]() {
            return !lockQueue.empty();
        });
        T temp = lockQueue.front();
        lockQueue.pop();
        return temp;  
    }
private:
    std::queue<T> lockQueue;
    std::mutex lockMutex;
    std::condition_variable lockCv;
};