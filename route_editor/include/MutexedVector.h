#ifndef MUTEXED_VECTOR_H
#define MUTEXED_VECTOR_H

#include <algorithm>
#include <mutex>
#include <vector>

template <class T>
class MutexedVector
{
public:
    using value_type = T;
    using reference = value_type&;

    bool empty()
    {
        std::lock_guard<std::mutex> lock{mutex_};
        return vector_.empty();
    }

    void clear()
    {
        std::lock_guard<std::mutex> lock{mutex_};
        vector_.clear();
    }

    template <class... Args>
    reference emplace_back(Args&&... args)
    {
        std::lock_guard<std::mutex> lock{mutex_};
        return vector_.emplace_back(args...);
    }

    template <class UnaryFunc>
    UnaryFunc for_each(UnaryFunc f)
    {
        std::lock_guard<std::mutex> lock{mutex_};
        return std::for_each(vector_.begin(), vector_.end(), f);
    }

private:
    std::vector<value_type> vector_;
    std::mutex mutex_;
};

#endif // MUTEXED_VECTOR_H
