#pragma once
#include <memory>

template <typename Key, typename Value>
class AbstractConsumer
{
public:
    virtual void consume(Key id, const Value& value) = 0;
};

template <typename Key, typename Value>
using ConsumerSharedPtr = std::shared_ptr<AbstractConsumer<Key, Value>>;
