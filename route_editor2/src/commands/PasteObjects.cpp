#include "editor/commands/PasteObjects.h"

#include "editor/commands/Command.h"
#include "editor/EditorContext.h"
#include "editor/Route.h"

#include <vsg/core/Mask.h>
#include <vsg/core/ref_ptr.h>
#include <vsg/nodes/Switch.h>

#include <algorithm>
#include <cstdio>

PasteObjects::PasteObjects(EditorContext& context)
    : Command(context)
{
    // Копии делаем в конструкторе: буфер обмена может измениться
    // до redo, а вставляться должны те же объекты
    for (const auto& object : context.selected_objects)
    {
        copied_objects_.emplace_back(object->copy());
    }

    update_description();
}

void PasteObjects::execute()
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

void PasteObjects::undo()
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

void PasteObjects::update_description()
{
    std::snprintf(description_, COMMAND_DESCRIPTION_BUFFER_SIZE,
        "Paste objects: %zu objects",
        copied_objects_.size()
    );
}
