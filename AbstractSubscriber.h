#pragma once
#include "AbstractConsumer.h"

template <typename Key, typename Value>
class AbstractSubscriber
{
public:
    virtual void subscribe(Key id, const ConsumerSharedPtr<Key, Value>& consumer) = 0;
    virtual void unsubscribe(Key id) = 0;
    virtual void unsubscribeAll() = 0;
};
