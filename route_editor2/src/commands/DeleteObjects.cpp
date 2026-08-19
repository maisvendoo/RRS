#include "editor/commands/DeleteObjects.h"

#include "editor/commands/Command.h"
#include "editor/EditorContext.h"
#include "editor/Route.h"

#include <vsg/core/Mask.h>
#include <vsg/nodes/Switch.h>

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

        {
            std::lock_guard<std::mutex> lock_guard(context_.static_objects_mutex);
            context_.static_objects.erase(
                std::find(context_.static_objects.begin(),
                    context_.static_objects.end(), object));
        }
        --context_.static_objects_count;
        --context_.total_static_objects_count;

        const vsg::ref_ptr<Route> route = context_.route;

        if (route)
        {
            route->children.erase(
                std::find_if(route->children.begin(), route->children.end(),
                    [object](const vsg::Switch::Child& child) {
                        return child.node == object;
                    }
                )
            );
        }
    }

    context_.compile_infos.emplace_back(CompileInfo{nullptr, context_.route});
}

void DeleteObjects::undo()
{
    for (const auto& object : objects_)
    {
        context_.compile_infos.emplace_back(CompileInfo{
            context_.route, object, vsg::MASK_ALL});

        {
            std::lock_guard<std::mutex> lock_guard(context_.static_objects_mutex);
            context_.static_objects.emplace_back(object);
        }

        ++context_.static_objects_count;
        ++context_.total_static_objects_count;

        object->select();
    }
}

void DeleteObjects::update_description()
{
    std::snprintf(description_, COMMAND_DESCRIPTION_BUFFER_SIZE,
        "Delete objects: %zu objects",
        objects_.size()
    );
}
