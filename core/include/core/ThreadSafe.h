#ifndef CORE_THREAD_SAFE_H
#define CORE_THREAD_SAFE_H

#include <mutex>

template <typename T>
class ThreadSafe
{
private:
    class ThreadSafeValue
    {
    public:
        ThreadSafeValue(T& value, std::mutex& mutex)
            : value(value)
            , mutex(mutex)
        {
            mutex.lock();
        }

        ~ThreadSafeValue()
        {
            mutex.unlock();
        }

        ThreadSafeValue(const ThreadSafeValue&) = delete;
        ThreadSafeValue& operator=(const ThreadSafeValue&) = delete;
        ThreadSafeValue(ThreadSafeValue&&) = delete;
        ThreadSafeValue& operator=(ThreadSafeValue&&) = delete;

        T& operator*() { return value; }
        const T& operator*() const { return value; }

        T* operator->() { return &value; }
        const T* operator->() const { return &value; }

    private:
        T& value;
        std::mutex& mutex;
    };

public:
    ThreadSafeValue lock()
    {
        return ThreadSafeValue(value, mutex);
    }

private:
    T value;
    std::mutex mutex;
};

#endif // CORE_THREAD_SAFE_H
