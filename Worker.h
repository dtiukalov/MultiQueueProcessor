#pragma once

#include <atomic>
#include <thread>
#include <future>
#include <functional>
#include <chrono>
#include <condition_variable>

#include "Queue.h"
#include "AbstractConsumer.h"

template <typename Key, typename Value>
class Worker {
public:
    Worker() {}

    Worker(const Worker&) = delete;
    Worker& operator=(const Worker&) = delete;

    Worker(Worker&& other) noexcept {
        std::exchange(m_que, other.m_que);
        std::exchange(m_consumer, other.m_consumer);
        m_thread = std::move(other.m_thread);
        m_working.exchange(other.m_working, std::memory_order_relaxed);
        m_id = other.m_id;
    }
    Worker& operator=(Worker&& other) noexcept
    {
        if (&other == this)
            return *this;

        m_que = other.m_que;
        other.m_que = nullptr; 

        m_consumer = other.m_consumer;
        other.m_consumer = nullptr;

        m_thread = std::move(other.m_thread);
        m_working.exchange(other.m_working, std::memory_order_relaxed);
        m_id = other.m_id;

        return *this;
    }

    ~Worker() {
        stopProcessing();  
        join();
        m_consumer = nullptr;
        m_que = nullptr;
    }

    void execute(Key id, const ConsumerSharedPtr<Key, Value>& consumer, Queue<Value>& queue) {
        bool executed = m_working.exchange(true, std::memory_order_relaxed);
        if (executed)
            return;

        m_id = id;
        m_que = &queue;
        m_consumer = consumer.get();

        m_thread = std::move(std::thread(std::bind(&Worker::process, this)));
    }

    void stopProcessing() {
        m_working.exchange(false, std::memory_order_relaxed);
    }

private:
    void process() {
        while (m_working) {
            if (m_que && m_consumer) {
                Value item(std::move(m_que->dequeue()));
                m_consumer->consume(m_id, item);
                std::this_thread::yield();
            }
        } 
    }

    void join() {
        if (m_thread.joinable())
            m_thread.join();
    }

    Key m_id;

    Queue<Value>* m_que = nullptr;
    AbstractConsumer<Key, Value>* m_consumer = nullptr;
    
    std::atomic<bool> m_working = false;
    std::thread m_thread;
};
