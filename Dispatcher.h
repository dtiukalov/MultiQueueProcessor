#pragma once

#include <mutex>
#include <cassert>
#include <iostream>
#include <limits>
#include <unordered_map>
#include <memory>

#include "AbstractConsumer.h"
#include "AbstractSubscriber.h"
#include "Queue.h"
#include "Worker.h"


template <typename Key, typename Value, size_t QueueMaxCapacity = std::numeric_limits<size_t>::max()>
class Dispatcher : public AbstractSubscriber<Key, Value>
{
public:
    Dispatcher() { }
    ~Dispatcher() {
        unsubscribeAll();
    }

    bool enqueue(Key id, Value value) {
        auto queueIter = m_queues.find(id);

        if (queueIter == m_queues.end()) {
            Queue<Value, QueueMaxCapacity> queue;
            queue.enqueue(std::move(value));
            m_queues.emplace(id, std::move(queue));
            return true;
        }
        else {
            auto& queue = queueIter->second;
            return queue.enqueue(std::move(value));
        }
    }

    void subscribe(Key id, const ConsumerSharedPtr<Key, Value>& consumer) override {
        std::lock_guard<std::mutex> locker(m_mtx);

        auto consumerIter = m_consumers.find(id);

        if (consumerIter == m_consumers.end()) {
            auto queueIter = m_queues.find(id);

            if (queueIter != m_queues.end()) {
                m_consumers.emplace(id, std::move(consumer));
                
                m_workers.emplace(id, std::move(Worker<Key, Value>()));
                m_workers[id].execute(id, consumer, queueIter->second);
            }
            else {
                std::cout << "The Queue with id #" + std::to_string(id) + " does not exist yet.\n";
            }
        }
    }

    void unsubscribe(Key id) override {
        std::lock_guard<std::mutex> locker(m_mtx);

        auto iter = m_consumers.find(id);
        if (iter != m_consumers.end())
            m_consumers.erase(iter);

        auto workerIter = m_workers.find(id);
        if (workerIter != m_workers.end()) {
            workerIter->second.stopProcessing();
            m_workers.erase(workerIter);
        }

        std::cout << "Unsubscribed from queue #" + std::to_string(id) + "\n";
    }

    void unsubscribeAll() override {
        std::lock_guard<std::mutex> locker(m_mtx);
        m_consumers.clear();
        for (auto& worker : m_workers) {
            worker.second.stopProcessing();
        }
        m_workers.clear();

        std::cout << "Unsubscribed from all queues\n";
	}

private:

    std::mutex m_mtx;
    std::unordered_map<Key, ConsumerSharedPtr<Key, Value>> m_consumers;
    std::unordered_map<Key, Queue<Value, QueueMaxCapacity>> m_queues;
    std::unordered_map<Key, Worker<Key, Value>> m_workers;
};
