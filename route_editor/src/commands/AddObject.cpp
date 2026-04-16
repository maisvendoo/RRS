#include "commands/AddObject.h"

#include "commands/Command.h"
#include "EditorContext.h"
#include "Gizmo.h"
#include "Route.h"

#include <vsg/core/Mask.h>
#include <vsg/core/ref_ptr.h>

#include <cstdio>

AddObject::AddObject(EditorContext& context,
    vsg::ref_ptr<RouteObject> object)
    : Command(context)
    , object_to_add_(object)
    , objects_to_deselect_(context.selected_objects)
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
        context_.route, object_to_add_, vsg::MASK_ALL});

    context_.static_objects.emplace_back(object_to_add_);

    context_.deferred_selection.emplace_back(object_to_add_);
}

void AddObject::undo()
{
    object_to_add_->deselect();

    RouteObjects& objects = context_.static_objects;
    objects.erase(std::find(objects.begin(), objects.end(), object_to_add_));

    const auto route = context_.route;

    for (auto it = route->children.begin(); it != route->children.end();
        ++it)
    {
        if (it->node == object_to_add_)
        {
            route->children.erase(it);
            break;
        }
    }

    for (const auto& object : objects_to_deselect_)
    {
        object->select();
    }

    context_.compile_infos.emplace_back(CompileInfo{nullptr, context_.route});

    context_.gizmo->update_visibility();
}

void AddObject::update_description()
{
    std::snprintf(description_, COMMAND_DESCRIPTION_BUFFER_SIZE,
        "Add object: \"%s\"", object_to_add_->label.c_str()
    );
}
