#include "editor/commands/DeleteObjectsCommand.h"

#include "editor/MergeSwitch.h"
#include "editor/commands/Command.h"
#include "editor/EditorContext.h"
#include "editor/Gizmo.h"
#include "editor/Route.h"
#include "editor/RouteObject.h"

#include <vsg/app/Viewer.h>
#include <vsg/core/Mask.h>
#include <vsg/core/observer_ptr.h>
#include <vsg/nodes/Switch.h>

#include <algorithm>
#include <cstdio>

DeleteObjectsCommand::DeleteObjectsCommand(EditorContext& editor_ctx)
    : Command(editor_ctx)
    , objects(editor_ctx.selected_objects)
{
    update_description();
}

void DeleteObjectsCommand::execute()
{
    for (const auto& object : objects)
    {
        object->deselect();

        editor_ctx.static_objects.lock()->remove(object);

        auto& route_children = editor_ctx.route->children;

        route_children.erase(
            std::remove_if(route_children.begin(), route_children.end(),
                [object](const vsg::Switch::Child& child) {
                    return child.node == object;
                }
            ),
            route_children.end()
        );
    }

    editor_ctx.compile_info_nodes.lock()->emplace_back(
        CompileInfoNode{editor_ctx.route});

    editor_ctx.gizmo->update_visibility();
}

void DeleteObjectsCommand::undo()
{
    auto static_objects = editor_ctx.static_objects.lock();

    auto& viewer = editor_ctx.viewer;
    vsg::observer_ptr<vsg::Viewer> observer_viewer(viewer);

    for (const auto& object : objects)
    {
        auto compile_result = viewer->compileManager->compile(object);
        viewer->addUpdateOperation(MergeSwitch::create(observer_viewer,
                                                       editor_ctx.route,
                                                       vsg::MASK_ALL, object,
                                                       compile_result));

        static_objects->emplace_back(object);

        object->select();
    }

    editor_ctx.gizmo->update_visibility();
}

void DeleteObjectsCommand::update_description()
{
    std::snprintf(description, COMMAND_DESCRIPTION_BUFFER_SIZE,
        "Delete objects: to delete: %zu objects", objects.size());
}
