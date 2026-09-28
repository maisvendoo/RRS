#include "editor/commands/AddObjectCommand.h"

#include "editor/Route.h"
#include "editor/RouteObject.h"
#include "editor/EditorContext.h"
#include "editor/Gizmo.h"
#include "editor/commands/Command.h"

#include <vsg/core/Mask.h>
#include <vsg/core/ref_ptr.h>

#include <algorithm>
#include <cstdio>

AddObjectCommand::AddObjectCommand(EditorContext& editor_context,
    const vsg::ref_ptr<RouteObject>& object)
    : Command(editor_context)
    , object_to_add_(object)
    , objects_to_deselect_(editor_context.selected_objects)
{
    update_description();
}

void AddObjectCommand::execute()
{
    for (const auto& object : objects_to_deselect_)
    {
        object->deselect();
    }

    editor_context.compile_infos.lock()->emplace_back(CompileInfo{
        editor_context.route, object_to_add_, vsg::MASK_ALL});

    editor_context.static_objects.lock()->emplace_back(object_to_add_);

    editor_context.deferred_selection.emplace_back(object_to_add_);
}

void AddObjectCommand::undo()
{
    object_to_add_->deselect();

    editor_context.static_objects.lock()->remove(object_to_add_);

    auto& route_children = editor_context.route->children;

    route_children.erase(
        std::remove_if(route_children.begin(), route_children.end(),
            [this](const vsg::Switch::Child& child) {
                return child.node == object_to_add_;
            }
        ),
        route_children.end()
    );

    for (const auto& object : objects_to_deselect_)
    {
        object->select();
    }

    editor_context.compile_infos.lock()->emplace_back(CompileInfo{
        nullptr, editor_context.route});

    editor_context.gizmo->update_visibility();
}

void AddObjectCommand::update_description()
{
    std::snprintf(description, COMMAND_DESCRIPTION_BUFFER_SIZE,
        "Add object: \"%s\"", object_to_add_->label.c_str()
    );
}
