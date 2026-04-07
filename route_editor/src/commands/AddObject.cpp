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
    , object_to_add(object)
    , objects_to_deselect(context.selected_objects)
{
    update_description();
}

void AddObject::execute()
{
    for (const auto& object : objects_to_deselect)
    {
        object->deselect();
    }

    context.compile_infos.emplace_back(CompileInfo{
        context.route, object_to_add, vsg::MASK_ALL});

    context.static_objects.emplace_back(object_to_add);

    context.deferred_selection.emplace_back(object_to_add);
}

void AddObject::undo()
{
    object_to_add->deselect();

    RouteObjects& objects = context.static_objects;
    objects.erase(std::find(objects.begin(), objects.end(), object_to_add));

    const auto route = context.route;

    for (auto it = route->children.begin(); it != route->children.end();
        ++it)
    {
        if (it->node == object_to_add)
        {
            route->children.erase(it);
            break;
        }
    }

    for (const auto& object : objects_to_deselect)
    {
        object->select();
    }

    context.compile_infos.emplace_back(CompileInfo{nullptr, context.route});

    context.gizmo->update_visibility();
}

void AddObject::update_description()
{
    std::snprintf(description, COMMAND_DESCRIPTION_BUFFER_SIZE,
        "Add object: \"%s\"", object_to_add->label.c_str()
    );
}
