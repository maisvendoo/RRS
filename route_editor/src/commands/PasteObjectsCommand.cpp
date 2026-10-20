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
    , objects_to_paste(context.copied_objects)
    , objects_to_deselect(context.selected_objects)
{
    update_description();
}

void PasteObjectsCommand::execute()
{
    for (const auto& object : objects_to_deselect)
        object->deselect();

    if (pasted_objects.empty())
    {
        for (const auto& object : objects_to_paste)
            pasted_objects.emplace_back(object->copy());
    }

    auto compile_infos = editor_ctx.compile_infos.lock();
    auto static_objects = editor_ctx.static_objects.lock();

    for (const auto& pasted_object : pasted_objects)
    {
        compile_infos->emplace_back(CompileInfo{
            editor_ctx.route, pasted_object, vsg::MASK_ALL});

        static_objects->emplace_back(pasted_object);

        pasted_object->select();
    }

    editor_ctx.gizmo->update_visibility();
}

void PasteObjectsCommand::undo()
{
    for (const auto& pasted_object : pasted_objects)
    {
        pasted_object->deselect();

        editor_ctx.static_objects.lock()->remove(pasted_object);

        auto& route_children = editor_ctx.route->children;

        route_children.erase(
            std::remove_if(route_children.begin(), route_children.end(),
                [pasted_object](const vsg::Switch::Child& child) {
                    return child.node == pasted_object;
                }
            ),
            route_children.end()
        );
    }

    for (const auto& object : objects_to_deselect)
        object->select();

    editor_ctx.compile_infos.lock()->emplace_back(CompileInfo{
        nullptr, editor_ctx.route});

    editor_ctx.gizmo->update_visibility();
}

void PasteObjectsCommand::update_description()
{
    std::snprintf(description, COMMAND_DESCRIPTION_BUFFER_SIZE,
        "Paste objects: to paste: %zu objects", objects_to_paste.size());
}
