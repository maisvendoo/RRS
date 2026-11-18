#include "editor/states/BasicEditorState.h"

#include "editor/Action.h"
#include "editor/Camera.h"
#include "editor/EditorContext.h"
#include "editor/Gizmo.h"
#include "editor/Keyboard.h"
#include "editor/Mask.h"
#include "editor/Mouse.h"
#include "editor/Route.h"
#include "editor/RouteObject.h"
#include "editor/StateManager.h"
#include "editor/commands/CommandManager.h"
#include "editor/commands/DeleteObjectsCommand.h"
#include "editor/commands/PasteObjectsCommand.h"
#include "editor/commands/SelectObjectsCommand.h"

#include "editor/StaticObjectManager.h"

#include <Journal.h>
#include <filesystem.h>

#include <vsg/ui/PointerEvent.h>
#include <vsg/utils/LineSegmentIntersector.h>
#include <vsgImGui/imgui.h>

BasicEditorState::BasicEditorState(EditorContext& editor_ctx)
    : State(editor_ctx)
    , keyboard(*editor_ctx.keyboard)
    , mouse(*editor_ctx.mouse)
    , camera(*editor_ctx.camera)
    , command_manager(*editor_ctx.command_manager)
    , static_object_manager(*editor_ctx.object_manager)
{
    name = "BasicEditorState";
}

BasicEditorState::~BasicEditorState() = default;

void BasicEditorState::handle_key_press()
{
    const auto& selected_objects = editor_ctx.selected_objects;
    auto& copied_objects = editor_ctx.copied_objects;
    const auto& state_manager = editor_ctx.state_manager;

    if (keyboard.pressed_once(ACTION_UNDO_COMMAND))
    {
        command_manager.undo();
    }
    else if (keyboard.pressed_once(ACTION_REDO_COMMAND))
    {
        command_manager.redo();
    }
    else if(keyboard.pressed_once(ACTION_SAVE_ROUTE))
    {
        save_route();
    }
    else if (keyboard.pressed_once(ACTION_SWAP_PROJECTION_MATRIX))
    {
        camera.swap_proj_mat();
    }
    else if (keyboard.pressed_once(ACTION_COPY_OBJECTS))
    {
        copied_objects = selected_objects;
    }
    else if (keyboard.pressed_once(ACTION_PASTE_OBJECTS))
    {
        auto command = std::make_unique<PasteObjectsCommand>(editor_ctx);
        command->execute();
        command_manager.push(std::move(command));
    }
    else if (keyboard.pressed(ACTION_DELETE_OBJECTS))
    {
        auto command = std::make_unique<DeleteObjectsCommand>(editor_ctx);
        command->execute();
        command_manager.push(std::move(command));
    }
    else
    {
        const bool pressed_action_move = keyboard.pressed_once(ACTION_TRANSLATE_OBJECTS);
        const bool pressed_action_rotate = keyboard.pressed_once(ACTION_ROTATE_OBJECTS);
        const bool pressed_action_scale = keyboard.pressed_once(ACTION_SCALE_OBJECTS);

        if (!pressed_action_move && !pressed_action_rotate && !pressed_action_scale)
        {
            return;
        }

        for (const auto& object : selected_objects)
        {
            object->save_matrix();
        }

        if (pressed_action_move)
        {
            state_manager->defer_switch_to(STATE_KEYBOARD_TRANSLATE);
        }
        else if (pressed_action_rotate)
        {
            state_manager->defer_switch_to(STATE_KEYBOARD_ROTATE);
        }
        else if (pressed_action_scale)
        {
            state_manager->defer_switch_to(STATE_KEYBOARD_SCALE);
        }
    }
}

void BasicEditorState::handle_button_press()
{
    const auto& state_manager = editor_ctx.state_manager;
    const auto& route = editor_ctx.route;
    const auto& gizmo = editor_ctx.gizmo;
    const auto& selected_objects = editor_ctx.selected_objects;

    switch (mouse.get_button_mask())
    {
        case vsg::BUTTON_MASK_1:
        {
            if (gizmo->handle_intersections())
            {
                // state_manager->defer_switch_to(STATE_GIZMO_TRANSLATE);
                return;
            }

            const auto intersector = vsg::LineSegmentIntersector::create(
                camera, mouse.get_x(), mouse.get_y());
            intersector->traversalMask = MASK_CLICKABLE;

            route->accept(*intersector);
            auto& intersections = intersector->intersections;
            if (intersections.empty())
            {
                if (selected_objects.empty() || keyboard.get_shift_state())
                {
                    return;
                }

                auto command = std::make_unique<SelectObjectsCommand>(editor_ctx);
                command->ids_to_deselect = static_object_manager.get_selected_objects().copy_shrinked_ids();
                command->update_description();
                command->execute();
                command_manager.push(std::move(command));
                return;
            }

            for (const vsg::Node* const node : intersections.front()->nodePath)
            {
                if (const auto* mat_transform = dynamic_cast<const vsg::MatrixTransform*>(node))
                {
                    select_object(mat_transform);
                    return;
                }
            }

            return;
        }
        case vsg::BUTTON_MASK_3:
        {
            state_manager->defer_switch_to(STATE_NAVIGATION);
            return;
        }
        default:
        {
            return;
        }
    }
}

void BasicEditorState::handle_mouse_scroll()
{
    camera.handle_mouse_scroll();
}

void BasicEditorState::save_route()
{
    const auto& route = editor_ctx.route;

    const auto& fs = FileSystem::getInstance();
    const std::string save_dir = fs.combinePath(route->route_dir,
        "topology", "map");

    try
    {
        // Создаём резервную копию
        std::filesystem::copy_file(fs.combinePath(save_dir, "route1.map"),
            fs.combinePath(save_dir, "route1.map.prev"),
            std::filesystem::copy_options::overwrite_existing);
    }
    catch (const std::filesystem::filesystem_error& e)
    {
        Journal::instance()->error(e.what());
    }

    // Перезаписываем рабочую копию
    std::ofstream route_map_file{fs.combinePath(save_dir, "route1.map")};

    auto static_objects = editor_ctx.static_objects.lock();
    for (const auto& object : *static_objects)
    {
        const vsg::dvec3& translation{object->get_position()};
        const vsg::dvec3 rotation_deg{-object->get_rot_deg()};

        route_map_file << object->label << "," <<
            translation.x << "," << translation.y << "," << translation.z << "," <<
            rotation_deg.x << "," << rotation_deg.y << "," << rotation_deg.z << ";\n";
    }
}

void BasicEditorState::select_object(const vsg::MatrixTransform* mat_transform)
{
    auto& selected_objects = static_object_manager.get_selected_objects();
    const std::size_t selected_objects_size = selected_objects.get_size();

    std::size_t object_id;
    static_object_manager.get_mat_transform_id(mat_transform, object_id);
    const bool is_selected = selected_objects.contains(object_id);

    auto command = std::make_unique<SelectObjectsCommand>(editor_ctx);

    if (keyboard.get_shift_state())
    {
        if (is_selected)
        {
            command->ids_to_deselect.emplace_back(object_id);
        }
        else
        {
            command->ids_to_select.emplace_back(object_id);
        }
    }
    else
    {
        if (selected_objects_size == 0)
        {
            command->ids_to_select.emplace_back(object_id);
        }
        else if (is_selected)
        {
            if (selected_objects_size == 1)
            {
                command->ids_to_deselect.emplace_back(object_id);
            }
            else
            {
                selected_objects.for_each([&](std::size_t id) -> void {
                    if (id != object_id)
                    {
                        command->ids_to_deselect.emplace_back(id);
                    }
                });
            }
        }
        else
        {
            command->ids_to_select.emplace_back(object_id);
            command->ids_to_deselect = selected_objects.copy_shrinked_ids();
        }
    }

    command->update_description();
    command->execute();

    command_manager.push(std::move(command));
}
