#pragma once
#include <iostream>
#include <mutex>
#include "AbstractConsumer.h"

class Consumer : public AbstractConsumer<int, int>
{
    std::mutex logMutex;
    std::condition_variable m_cond;
public:
    void consume(int id, const int& value) override {
        std::unique_lock<std::mutex> locker(logMutex);
        std::cout << "Queue id - " << id << "; value - " << value << "\n";
        locker.unlock();
        m_cond.notify_one();
    }
};
