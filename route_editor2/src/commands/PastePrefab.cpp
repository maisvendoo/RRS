#include "editor/commands/PastePrefab.h"

#include "editor/commands/Command.h"
#include "editor/EditorContext.h"
#include "editor/Route.h"

#include <vsg/core/Mask.h>
#include <vsg/core/ref_ptr.h>
#include <vsg/nodes/Switch.h>

#include <algorithm>
#include <cstdio>
#include <utility>

PastePrefab::PastePrefab(EditorContext& context,
    RouteObjects copied_objects, const vsg::dvec3& offset,
    const std::string& prefab_name)
    : Command(context)
    , copied_objects_(std::move(copied_objects))
    , offset_(offset)
    , prefab_name_(prefab_name)
{
    update_description();
}

void PastePrefab::execute()
{
    for (const auto& object : copied_objects_)
    {
        object->move(offset_);

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

    context_.selected_objects = copied_objects_;
}

void PastePrefab::undo()
{
    for (const auto& object : copied_objects_)
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

    context_.selected_objects.clear();
    context_.compile_infos.emplace_back(CompileInfo{nullptr, context_.route});
}

void PastePrefab::update_description()
{
    std::snprintf(description_, COMMAND_DESCRIPTION_BUFFER_SIZE,
        "Paste prefab \"%s\": %zu objects",
        prefab_name_.c_str(),
        copied_objects_.size()
    );
}
