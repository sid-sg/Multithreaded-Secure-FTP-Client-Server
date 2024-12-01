#ifndef THREAD_POOL
#define THREAD_POOL

#include <condition_variable>
#include <functional>
#include <iostream>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

class Threadpool {
   private:
    int numThreads;
    std::vector<std::thread> threads;
    std::queue<std::function<void()>> tasks;
    std::mutex mtx;
    std::condition_variable cv;
    bool stop;

   public:
    explicit Threadpool(int numThreads);
    void enqueueTask(std::function<void()> func);
    ~Threadpool();
};

Threadpool::Threadpool(int numThreads) : numThreads(numThreads), stop(false) {
    for (int i = 0; i < numThreads; i++) {
        threads.emplace_back([this]() {
            std::function<void()> task;
            while (1) {
                std::unique_lock<std::mutex> lock(mtx);
                cv.wait(lock, [this]() { return !tasks.empty() || stop; });

                if (stop) {
                    return;
                }
                task = std::move(tasks.front());
                tasks.pop();

                lock.unlock();
                task();
            }
        });
    }
    std::cout << "Threadpool created\n";
}

Threadpool::~Threadpool() {
    std::unique_lock<std::mutex> lock(mtx);
    stop = true;
    lock.unlock();
    cv.notify_all();

    for (auto& t : threads) {
        t.join();
    }
}

void Threadpool::enqueueTask(std::function<void()> func) {
    std::unique_lock<std::mutex> lock(mtx);
    tasks.push(func);

    lock.unlock();
    cv.notify_one();
}

#endif