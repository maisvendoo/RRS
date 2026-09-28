#include "editor/commands/PasteObjectsCommand.h"

#include "editor/EditorContext.h"
#include "editor/Gizmo.h"
#include "editor/Route.h"
#include "editor/RouteObject.h"
#include "editor/commands/Command.h"

#include <vsg/core/Mask.h>
#include <vsg/nodes/Switch.h>

#include <algorithm>
#include <cstdio>

PasteObjectsCommand::PasteObjectsCommand(
    EditorContext& context
)
    : Command(context)
    , objects_to_paste_(context.copied_objects)
    , objects_to_deselect_(context.selected_objects)
{
    update_description();
}

void PasteObjectsCommand::execute()
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

    auto compile_infos = editor_context.compile_infos.lock();
    auto static_objects = editor_context.static_objects.lock();

    for (const auto& pasted_object : pasted_objects_)
    {
        compile_infos->emplace_back(CompileInfo{
            editor_context.route, pasted_object, vsg::MASK_ALL});

        static_objects->emplace_back(pasted_object);

        pasted_object->select();
    }

    editor_context.gizmo->update_visibility();
}

void PasteObjectsCommand::undo()
{
    for (const auto& pasted_object : pasted_objects_)
    {
        pasted_object->deselect();

        editor_context.static_objects.lock()->remove(pasted_object);

        auto& route_children = editor_context.route->children;

        route_children.erase(
            std::remove_if(route_children.begin(), route_children.end(),
                [pasted_object](const vsg::Switch::Child& child) {
                    return child.node == pasted_object;
                }
            ),
            route_children.end()
        );
    }

    for (const auto& object : objects_to_deselect_)
    {
        object->select();
    }

    editor_context.compile_infos.lock()->emplace_back(CompileInfo{
        nullptr, editor_context.route});

    editor_context.gizmo->update_visibility();
}

void PasteObjectsCommand::update_description()
{
    std::snprintf(description, COMMAND_DESCRIPTION_BUFFER_SIZE,
        "Paste objects: to paste: %zu objects",
        objects_to_paste_.size()
    );
}
