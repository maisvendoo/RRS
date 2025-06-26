#include "animations-list.h"

#include <vsg/core/ref_ptr.h>

#include <cstddef>
#include <map>
#include <mutex>
#include <utility>

class ProcAnimation;

void animations_t::thread_safe_insert(std::pair<std::size_t, vsg::ref_ptr<ProcAnimation>> id_and_animation)
{
    std::lock_guard lock(mutex);
    animations.insert(id_and_animation);
}
