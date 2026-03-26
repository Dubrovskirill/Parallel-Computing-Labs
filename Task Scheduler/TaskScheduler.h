#pragma once
#include <iostream>
#include <vector>
#include <functional>
#include <future>
#include "D:\Учеба\4 курс\Основы распределенных вычислений\лабы\Thread-safety queue\Queue.h"

class TaskScheduler {
public:
    explicit TaskScheduler(size_t num_threads) {
        for (size_t i = 0; i < num_threads; ++i) {
            m_workers.emplace_back([this] {
                worker_loop();
                });
        }
    }

    ~TaskScheduler() {
        stop();
    }

    template <typename F, typename... Args>
    void submit(F&& f, Args&&... args) {
        auto task = std::bind(std::forward<F>(f), std::forward<Args>(args)...);
        m_task_queue.push(std::function<void()>(task));
    }

    void stop() {
        m_task_queue.set_finished(); 
        for (std::thread& worker : m_workers) {
            if (worker.joinable()) {
                worker.join(); 
            }
        }
        m_workers.clear();
    }

private:
    void worker_loop() {
        std::function<void()> task;
        while (m_task_queue.wait_and_pop(task)) {
            try {
                if (task) {
                    task(); 
                }
            }
            catch (const std::exception& e) {
                std::cerr << "Worker thread caught an exception: " << e.what() << std::endl;
            }
            catch (...) {
                std::cerr << "Worker thread caught an unknown error." << std::endl;
            }
        }
    }

    std::vector<std::thread> m_workers;
    Queue<std::function<void()>> m_task_queue;
};