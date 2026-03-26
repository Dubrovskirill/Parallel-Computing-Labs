#pragma once
#include <thread>
#include <mutex>
#include <condition_variable>

template <typename T>
class Queue {
private:
    struct Node {
        T data;
        Node* next;
        Node(T val) : data(std::move(val)), next(nullptr) {}
    };

    Node* m_front; 
    Node* m_back;  
    size_t m_count;
    std::mutex m_mtx;
    std::condition_variable m_cv;
    bool m_finished;


public:
    Queue() : m_front(nullptr), m_back(nullptr), m_count(0), m_finished(false) {}

    ~Queue() {
        while (m_front) {
            Node* temp = m_front;
            m_front = m_front->next;
            delete temp;
        }
    }


    void push(T value) {
        std::lock_guard<std::mutex> lock(m_mtx);

        Node* newNode = new Node(std::move(value));
        if (m_front == nullptr) {
            m_front = m_back = newNode;
        }
        else {
            m_back->next = newNode;
            m_back = newNode;
        }
        m_count++;
        m_cv.notify_one();
    }


    bool pop(T& value) {
        std::lock_guard<std::mutex> lock(m_mtx);

        if (m_front == nullptr) {
            return false;
        }

        value = std::move(m_front->data);
        Node* temp = m_front;
        m_front = m_front->next;
        delete temp;

        if (m_front == nullptr) {
            m_back = nullptr;
        }
        m_count--;
        return true;
    }


    // возвращает false только если очередь пуста и вызван set_finished().
    bool wait_and_pop(T& value) {
        std::unique_lock<std::mutex> lock(m_mtx);
        m_cv.wait(lock, [this] {
            return m_front != nullptr || m_finished;
            });

        if (m_front == nullptr && m_finished) {
            return false;
        }
        value = std::move(m_front->data);
        Node* temp = m_front;
        m_front = m_front->next;
        delete temp;

        if (m_front == nullptr) {
            m_back = nullptr;
        }
        m_count--;
        return true;
    }

    void set_finished() {
        {
            std::lock_guard<std::mutex> lock(m_mtx);
            m_finished = true;
        }
        // будим все потоки, чтобы они вышли из wait_and_pop
        m_cv.notify_all();
    }

    bool isEmpty() {
        std::lock_guard<std::mutex> lock(m_mtx);
        return m_front == nullptr;
    }

    size_t size() {
        std::lock_guard<std::mutex> lock(m_mtx);
        return m_count;
    }




};
//энтони хильямс
//паралл прог на с++ в действиях
