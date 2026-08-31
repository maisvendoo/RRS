#ifndef THREAD_SAFE_VECTOR_H
#define THREAD_SAFE_VECTOR_H

#include <algorithm>
#include <mutex>
#include <vector>

template <class T>
class ThreadSafeVector
{
public:
    using value_type = typename std::vector<T>::value_type;
    using size_type = typename std::vector<T>::size_type;
    using reference = typename std::vector<T>::reference;
    using iterator = typename std::vector<T>::iterator;

    reference at(size_type pos)
    {
        std::lock_guard<std::mutex> lock(mutex);
        return vector.at(pos);
    }

    reference operator[](size_type pos)
    {
        std::lock_guard<std::mutex> lock(mutex);
        return vector[pos];
    }

    reference front()
    {
        std::lock_guard<std::mutex> lock(mutex);
        return vector.front();
    }

    reference back()
    {
        std::lock_guard<std::mutex> lock(mutex);
        return vector.back();
    }

    iterator begin()
    {
        std::lock_guard<std::mutex> lock(mutex);
        return vector.begin();
    }

    iterator end()
    {
        std::lock_guard<std::mutex> lock(mutex);
        return vector.end();
    }

    iterator rbegin()
    {
        std::lock_guard<std::mutex> lock(mutex);
        return vector.rbegin();
    }

    iterator rend()
    {
        std::lock_guard<std::mutex> lock(mutex);
        return vector.rend();
    }

    bool empty()
    {
        std::lock_guard<std::mutex> lock(mutex);
        return vector.empty();
    }

    size_type size()
    {
        std::lock_guard<std::mutex> lock(mutex);
        return vector.size();
    }

    size_type max_size()
    {
        std::lock_guard<std::mutex> lock(mutex);
        return vector.max_size();
    }

    void reserve(size_type new_cap)
    {
        std::lock_guard<std::mutex> lock(mutex);
        vector.reserve(new_cap);
    }

    size_type capacity()
    {
        std::lock_guard<std::mutex> lock(mutex);
        return vector.capacity();
    }

    void shrink_to_fit()
    {
        std::lock_guard<std::mutex> lock(mutex);
        vector.shrink_to_fit();
    }

    void clear()
    {
        std::lock_guard<std::mutex> lock(mutex);
        vector.clear();
    }

    void push_back(const T& value)
    {
        std::lock_guard<std::mutex> lock(mutex);
        vector.push_back(value);
    }

    void push_back(T&& value)
    {
        std::lock_guard<std::mutex> lock(mutex);
        vector.push_back(value);
    }

    template <class... Args>
    reference emplace_back(Args&&... args)
    {
        std::lock_guard<std::mutex> lock(mutex);
        return vector.emplace_back(args...);
    }

    template <class UnaryFunc>
    UnaryFunc for_each(UnaryFunc f)
    {
        std::lock_guard<std::mutex> lock(mutex);
        return std::for_each(vector.begin(), vector.end(), f);
    }

private:
    std::vector<value_type> vector;
    std::mutex mutex;
};

#endif // THREAD_SAFE_VECTOR_H
