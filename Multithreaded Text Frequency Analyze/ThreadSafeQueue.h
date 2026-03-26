#pragma once
#include <queue>
#include <mutex>
#include <condition_variable>
#include <optional>
#include <string>

template <typename T>
class ThreadSafeQueue {
private:
    std::queue<T> m_queue;           
    std::mutex m_mtx;             
    std::condition_variable m_cond;  
    bool m_finished = false;         

public:
   
    void push(T value) {
        {
            std::lock_guard<std::mutex> lock(m_mtx);
            m_queue.push(std::move(value));
        }
        m_cond.notify_one();
    }

 
    bool pop(T& value) {
        std::unique_lock<std::mutex> lock(m_mtx);

        m_cond.wait(lock, [this] {
            return !m_queue.empty() || m_finished;
            });

        if (m_queue.empty()) {
            return false; 
        }

        value = std::move(m_queue.front());
        m_queue.pop();
        return true;
    }

   
    void set_finished() {
        {
            std::lock_guard<std::mutex> lock(m_mtx);
            m_finished = true;
        }
        m_cond.notify_all(); 
    }
};
