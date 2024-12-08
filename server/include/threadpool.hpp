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
    auto enqueueTask(F&& func, Args&&... args) -> std::future<decltype(func(args...))> {
        using returnType = decltype(func(args...));
        using taskType = std::packaged_task<returnType()>;
        auto boundTask = std::bind(std::forward<F>(func), std::forward<Args>(args)...);
        auto task = std::make_shared<taskType>(std::move(boundTask));

        std::future<returnType> result = task->get_future();

        std::unique_lock<std::mutex> lock(mtx);
        tasks.emplace([task]() -> void { (*task)(); });

        lock.unlock();
        cv.notify_one();
        return result;
    }
};

#endif