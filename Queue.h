#pragma once

#include <mutex>
#include <queue>
#include <condition_variable>
#include <limits>

template <typename Value, size_t MaxCapacity = std::numeric_limits<size_t>::max()>
class Queue
{
public:
    Queue() {}

    Queue(const Queue&) = delete;
    Queue& operator=(const Queue&) = delete;

    Queue(Queue&& other) noexcept {
        std::exchange(m_que, other.m_que);
    }
    Queue& operator=(Queue&& other) noexcept
    {
        if (&other == this)
            return *this;

        std::exchange(m_que, other.m_que);
        
        return *this;
    }

    bool enqueue(Value&& value) {
        std::unique_lock<std::mutex> locker(m_mtx);

        if (m_que.size() >= MaxCapacity)
            return false;

		m_que.push(std::move(value));

		locker.unlock();
		m_cond.notify_all();

        return true;
    }

    Value dequeue() {
        std::unique_lock<std::mutex> locker(m_mtx);
		m_cond.wait(locker, [this] () { return !m_que.empty(); });

		Value item(std::move(m_que.front()));
		m_que.pop();

		locker.unlock();
		m_cond.notify_one();

        return std::move(item);
    }

    bool isEmpty() {
        std::lock_guard<std::mutex> locker(m_mtx);
        return m_que.empty();
    }

private:
    std::mutex m_mtx;
    std::condition_variable m_cond;
	std::queue<Value> m_que;
};
