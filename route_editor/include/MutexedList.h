#ifndef MUTEXED_LIST_H
#define MUTEXED_LIST_H

#include <list>
#include <mutex>

template <class T>
class MutexedList
{
public:
    using value_type = T;

private:
    std::list<value_type> list_;
    std::mutex mutex_;
};

#endif // MUTEXED_LIST_H
