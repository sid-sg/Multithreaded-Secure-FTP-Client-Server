#ifndef THREAD_POOL
#define THREAD_POOL

#include <condition_variable>
#include <functional>
#include <future>
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
    ~Threadpool();

    template <typename F, typename... Args>
    auto enqueueTask(F&& func, Args&&... args) -> std::future<decltype(func(args...))>;
};

Threadpool::Threadpool(int numThreads) : numThreads(numThreads), stop(false) {
    if (numThreads <= 0) {
        throw std::invalid_argument("No. of threads must be >= 0");
    }
    for (int i = 0; i < numThreads; i++) {
        threads.emplace_back([this]() {
            std::function<void()> task;
            while (1) {
                std::unique_lock<std::mutex> lock(mtx);
                cv.wait(lock, [this]() { return !tasks.empty() || stop; });

                if (stop && tasks.empty()) {
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

template <typename F, typename... Args>
auto Threadpool::enqueueTask(F&& func, Args&&... args) -> std::future<decltype(func(args...))> {

    using returnType = decltype(func(args...));
    using taskType = std::packaged_task<returnType()>;
    auto boundTask = std::bind(std::forward<F>(func), std::forward<Args>(args)...);
    auto task = std::make_shared<taskType>(std::move(boundTask));

    std::future<returnType> result = task->get_future();

    std::unique_lock<std::mutex> lock(mtx);
    tasks.emplace([task]() -> void { 
        (*task)(); 
    });

    lock.unlock();
    cv.notify_one();
    return result;
}

#endif