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

#include <Journal.h>
#include <filesystem.h>

#include <vsg/ui/PointerEvent.h>
#include <vsg/utils/LineSegmentIntersector.h>
#include <vsgImGui/imgui.h>

BasicEditorState::BasicEditorState(EditorContext& editor_context)
    : State(editor_context)
{
    name = "BasicEditorState";
}

BasicEditorState::~BasicEditorState() = default;

void BasicEditorState::handle_key_press()
{
    const auto& actions = editor_context.actions;

    if (actions.active_action < 0)
    {
        return;
    }

    switch (actions.active_action)
    {
        case ACTION_UNDO_COMMAND:
        {
            editor_context.command_manager->undo();
            return;
        }
        case ACTION_REDO_COMMAND:
        {
            editor_context.command_manager->redo();
            return;
        }
        case ACTION_SAVE_ROUTE:
        {
            save_route();
            return;
        }
        case ACTION_SWAP_PROJECTION_MATRIX:
        {
            editor_context.camera->swap_projection_matrix();
            return;
        }
        case ACTION_COPY_OBJECTS:
        {
            editor_context.copied_objects = editor_context.selected_objects;
            return;
        }
        case ACTION_PASTE_OBJECTS:
        {
            auto command = std::make_unique<PasteObjectsCommand>(editor_context);
            command->execute();
            editor_context.command_manager->push(std::move(command));
            return;
        }
        case ACTION_DELETE_OBJECTS:
        {
            auto command = std::make_unique<DeleteObjectsCommand>(editor_context);
            command->execute();
            editor_context.command_manager->push(std::move(command));
            return;
        }
        case ACTION_TRANSLATE_OBJECTS:
        {
            for (const auto& object : editor_context.selected_objects)
            {
                object->save_matrix();
            }
            editor_context.state_manager->defer_switch_to(STATE_KEYBOARD_TRANSLATE);
            return;
        }
        case ACTION_ROTATE_OBJECTS:
        {
            for (const auto& object : editor_context.selected_objects)
            {
                object->save_matrix();
            }
            editor_context.state_manager->defer_switch_to(STATE_KEYBOARD_ROTATE);
            return;
        }
        case ACTION_SCALE_OBJECTS:
        {
            for (const auto& object : editor_context.selected_objects)
            {
                object->save_matrix();
            }
            editor_context.state_manager->defer_switch_to(STATE_KEYBOARD_SCALE);
            return;
        }
        default:
        {
            return;
        }
    }
}

void BasicEditorState::handle_button_press()
{
    const auto& mouse = editor_context.mouse;
    const auto& gizmo = editor_context.gizmo;
    const auto& camera = editor_context.camera;
    const auto& route = editor_context.route;
    const auto& selected_objects = editor_context.selected_objects;
    const auto& keyboard = editor_context.keyboard;
    const auto& command_manager = editor_context.command_manager;
    const auto& state_manager = editor_context.state_manager;

    switch (mouse->get_button_mask())
    {
        case vsg::BUTTON_MASK_1:
        {
            if (gizmo->handle_intersections())
            {
                return;
            }

            const auto intersector = vsg::LineSegmentIntersector::create(
                *camera, mouse->get_x(), mouse->get_y());
            intersector->traversalMask = MASK_CLICKABLE;

            route->accept(*intersector);
            auto& intersections = intersector->intersections;
            if (intersections.empty())
            {
                if (selected_objects.empty() || keyboard->get_shift_state())
                {
                    return;
                }

                auto command = std::make_unique<SelectObjectsCommand>(editor_context);
                command->objects_to_deselect = selected_objects;
                command->update_description();
                command->execute();
                command_manager->push(std::move(command));
                return;
            }

            for (const vsg::Node* const node : intersections.front()->nodePath)
            {
                if (const RouteObject* const object = node->cast<RouteObject>())
                {
                    select_object(vsg::ref_ptr(const_cast<RouteObject*>(object)));
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
    const auto& camera = editor_context.camera;

    camera->handle_mouse_scroll();
}

void BasicEditorState::save_route()
{
    const auto& route = editor_context.route;

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

    auto static_objects = editor_context.static_objects.lock();
    for (const auto& object : *static_objects)
    {
        const vsg::dvec3& translation{object->get_translation()};
        const vsg::dvec3 rotation_deg{-object->get_rotation_deg()};

        route_map_file << object->label << "," <<
            translation.x << "," << translation.y << "," << translation.z << "," <<
            rotation_deg.x << "," << rotation_deg.y << "," << rotation_deg.z << ";\n";
    }
}

void BasicEditorState::select_object(const vsg::ref_ptr<RouteObject>& object)
{
    const auto& keyboard = editor_context.keyboard;
    const auto& selected_objects = editor_context.selected_objects;
    const auto& command_manager = editor_context.command_manager;

    auto command = std::make_unique<SelectObjectsCommand>(editor_context);

    if (keyboard->get_shift_state())
    {
        if (object->get_is_selected())
        {
            command->objects_to_deselect.emplace_back(object);
        }
        else
        {
            command->objects_to_select.emplace_back(object);
        }
    }
    else
    {
        if (selected_objects.empty())
        {
            command->objects_to_select.emplace_back(object);
        }
        else if (object->get_is_selected())
        {
            if (selected_objects.size() == 1)
            {
                command->objects_to_deselect.emplace_back(object);
            }
            else
            {
                for (const auto& selected_object : selected_objects)
                {
                    if (selected_object != object)
                    {
                        command->objects_to_deselect.emplace_back(selected_object);
                    }
                }
            }
        }
        else
        {
            command->objects_to_select.emplace_back(object);
            command->objects_to_deselect = selected_objects;
        }
    }

    command->update_description();
    command->execute();

    command_manager->push(std::move(command));
}
