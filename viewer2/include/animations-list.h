#ifndef ANIMATIONS_LIST_H
#define ANIMATIONS_LIST_H

#include <cstddef>
#include <map>
#include <mutex>

class ProcAnimation;

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
struct animations_t
{
    animations_t(){}

    std::mutex mutex;
    std::multimap<std::size_t, ProcAnimation*> animations = {};

    void thread_safe_insert(std::pair<std::size_t, ProcAnimation*> id_and_animation)
    {
        std::lock_guard<std::mutex> lock(mutex);
        animations.insert(id_and_animation);
    }
};

#endif // ANIMATIONS_LIST_H
