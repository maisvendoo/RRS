#include "commands/DeleteObjects.h"

#include "commands/Command.h"
#include "EditorContext.h"
#include "Gizmo.h"
#include "Route.h"
#include "RouteObject.h"

#include <vsg/core/Mask.h>
#include <vsg/nodes/Switch.h>

#include <algorithm>
#include <cstdio>

DeleteObjects::DeleteObjects(
    EditorContext& context,
    const vsg::ref_ptr<Route>& route,
    const vsg::ref_ptr<Gizmo>& gizmo
)
    : Command(context)
    , objects_(context.selected_objects)
    , route(route)
    , gizmo(gizmo)
{
    update_description();
}

void DeleteObjects::execute()
{
    for (const auto& object : objects_)
    {
        object->deselect();

        context_.static_objects_mutex.lock();
        context_.static_objects.erase(std::find(context_.static_objects.begin(),
            context_.static_objects.end(), object));

        context_.static_objects_mutex.unlock();
        --context_.static_objects_count;
        --context_.total_static_objects_count;

        route->children.erase(
            std::find_if(route->children.begin(), route->children.end(),
                [object](const vsg::Switch::Child& child) {
                    return child.node == object;
                }
            )
        );
    }

    context_.compile_infos.emplace_back(CompileInfo{nullptr, route});

    gizmo->update_visibility();
}

void DeleteObjects::undo()
{
    for (const auto& object : objects_)
    {
        context_.compile_infos.emplace_back(CompileInfo{
            route, object, vsg::MASK_ALL});

        context_.static_objects_mutex.lock();
        context_.static_objects.emplace_back(object);
        context_.static_objects_mutex.unlock();

        ++context_.static_objects_count;
        ++context_.total_static_objects_count;

        object->select();
    }

    gizmo->update_visibility();
}

void DeleteObjects::update_description()
{
    std::snprintf(description_, COMMAND_DESCRIPTION_BUFFER_SIZE,
        "Delete objects: to delete: %zu objects",
        objects_.size()
    );
}
