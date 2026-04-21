#include "commands/DeleteObjects.h"

#include "commands/Command.h"
#include "EditorContext.h"
#include "Gizmo.h"
#include "Route.h"

#include <vsg/core/Mask.h>

#include <algorithm>
#include <cstdio>

DeleteObjects::DeleteObjects(EditorContext& context)
    : Command(context)
    , objects_(context.selected_objects)
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

        const auto route = context_.route;

        for (auto it = route->children.begin(); it != route->children.end();
            ++it)
        {
            if (it->node == object)
            {
                route->children.erase(it);
                break;
            }
        }
    }

    context_.compile_infos_mutex.lock();
    context_.compile_infos.emplace_back(CompileInfo{nullptr, context_.route});
    context_.compile_infos_mutex.unlock();

    context_.gizmo->update_visibility();
}

void DeleteObjects::undo()
{
    for (const auto& object : objects_)
    {
        context_.compile_infos_mutex.lock();
        context_.compile_infos.emplace_back(CompileInfo{
            context_.route, object, vsg::MASK_ALL});
        context_.compile_infos_mutex.unlock();

        context_.static_objects_mutex.lock();
        context_.static_objects.emplace_back(object);
        context_.static_objects_mutex.unlock();

        ++context_.static_objects_count;
        ++context_.total_static_objects_count;

        object->select();
    }

    context_.gizmo->update_visibility();
}

void DeleteObjects::update_description()
{
    std::snprintf(description_, COMMAND_DESCRIPTION_BUFFER_SIZE,
        "Delete objects: to delete: %zu objects",
        objects_.size()
    );
}
