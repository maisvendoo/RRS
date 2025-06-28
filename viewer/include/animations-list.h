#pragma once
#ifndef ANIMATIONS_LIST_H
#define ANIMATIONS_LIST_H

#include "ProcAnimation.h"

#include <vsg/core/Inherit.h>
#include <vsg/core/Object.h>
#include <vsg/core/ref_ptr.h>

#include <cstddef>
#include <map>
#include <mutex>
#include <utility>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
struct animations_t final : public vsg::Inherit<vsg::Object, animations_t>
{
    std::mutex mutex;
    std::multimap<std::size_t, vsg::ref_ptr<ProcAnimation>> animations;

    void thread_safe_insert(std::pair<std::size_t, vsg::ref_ptr<ProcAnimation>> id_and_animation);
};

#endif // ANIMATIONS_LIST_H
