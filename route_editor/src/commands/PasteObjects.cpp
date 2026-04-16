#include "commands/PasteObjects.h"

#include "commands/Command.h"
#include "EditorContext.h"
#include "Gizmo.h"
#include "Route.h"
#include "RouteObject.h"

#include <vsg/core/Mask.h>

#include <algorithm>
#include <cstdio>

PasteObjects::PasteObjects(EditorContext& context)
    : Command(context)
    , objects_to_paste_(context.copied_objects)
    , objects_to_deselect_(context.selected_objects)
{
    update_description();
}

void PasteObjects::execute()
{
    for (const auto& object : objects_to_deselect_)
    {
        object->deselect();
    }

    if (pasted_objects_.empty())
    {
        for (const auto& object : objects_to_paste_)
        {
            pasted_objects_.emplace_back(object->copy());
        }
    }

    for (const auto& pasted_object : pasted_objects_)
    {
        context_.compile_infos_mutex.lock();
        context_.compile_infos.emplace_back(CompileInfo{
            context_.route, pasted_object, vsg::MASK_ALL});
        context_.compile_infos_mutex.unlock();

        context_.static_objects_mutex.lock();
        context_.static_objects.emplace_back(pasted_object);
        context_.static_objects_mutex.unlock();

        pasted_object->select();
    }

    context_.gizmo->update_visibility();
}

void PasteObjects::undo()
{
    for (const auto& pasted_object : pasted_objects_)
    {
        pasted_object->deselect();

        context_.static_objects_mutex.lock();
        RouteObjects& objects = context_.static_objects;
        objects.erase(std::find(objects.begin(), objects.end(), pasted_object));
        context_.static_objects_mutex.unlock();

        const auto route = context_.route;

        for (auto it = route->children.begin(); it != route->children.end();
            ++it)
        {
            if (it->node == pasted_object)
            {
                route->children.erase(it);
                break;
            }
        }
    }

    for (const auto& object : objects_to_deselect_)
    {
        object->select();
    }

    context_.compile_infos_mutex.lock();
    context_.compile_infos.emplace_back(CompileInfo{nullptr, context_.route});
    context_.compile_infos_mutex.unlock();

    context_.gizmo->update_visibility();
}

void PasteObjects::update_description()
{
    std::snprintf(description_, COMMAND_DESCRIPTION_BUFFER_SIZE,
        "Paste objects: to paste: %zu objects",
        objects_to_paste_.size()
    );
}
