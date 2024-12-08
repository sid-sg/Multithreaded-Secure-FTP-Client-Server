#include "threadpool.hpp"

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
