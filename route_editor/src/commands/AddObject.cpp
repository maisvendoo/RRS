#include "commands/AddObject.h"

#include "Route.h"
#include "RouteObject.h"
#include "EditorContext.h"
#include "Gizmo.h"
#include "commands/Command.h"

#include <vsg/core/Mask.h>
#include <vsg/core/ref_ptr.h>

#include <algorithm>
#include <cstdio>

AddObject::AddObject(
    EditorContext& context,
    vsg::ref_ptr<RouteObject> object,
    const vsg::ref_ptr<Route>& route,
    const vsg::ref_ptr<Gizmo>& gizmo
)
    : Command(context)
    , object_to_add_(object)
    , objects_to_deselect_(context.selected_objects)
    , route(route)
    , gizmo(gizmo)
{
    update_description();
}

void AddObject::execute()
{
    for (const auto& object : objects_to_deselect_)
    {
        object->deselect();
    }

    context_.compile_infos.emplace_back(CompileInfo{
        route, object_to_add_, vsg::MASK_ALL});

    context_.static_objects_mutex.lock();
    context_.static_objects.emplace_back(object_to_add_);
    context_.static_objects_mutex.unlock();
    ++context_.static_objects_count;
    ++context_.total_static_objects_count;

    context_.deferred_selection.emplace_back(object_to_add_);
}

void AddObject::undo()
{
    object_to_add_->deselect();

    context_.static_objects_mutex.lock();
    RouteObjects& objects = context_.static_objects;
    objects.erase(std::find(objects.begin(), objects.end(), object_to_add_));
    context_.static_objects_mutex.unlock();

    --context_.static_objects_count;
    --context_.total_static_objects_count;

    route->children.erase(
        std::find_if(route->children.begin(), route->children.end(),
            [this](const vsg::Switch::Child& child) {
                return child.node == object_to_add_;
            }
        )
    );

    for (const auto& object : objects_to_deselect_)
    {
        object->select();
    }

    context_.compile_infos.emplace_back(CompileInfo{nullptr, route});

    gizmo->update_visibility();
}

void AddObject::update_description()
{
    std::snprintf(description_, COMMAND_DESCRIPTION_BUFFER_SIZE,
        "Add object: \"%s\"", object_to_add_->label.c_str()
    );
}
