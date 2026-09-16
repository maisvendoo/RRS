#include "editor/ObjectSelector.h"

#include "editor/EditorContext.h"
#include "editor/IntersectionHandler.h"
#include "editor/MouseButton.h"
#include "editor/RouteObject.h"
#include "editor/commands/CommandList.h"
#include "editor/commands/SelectObjects.h"

#include <vsgImGui/imgui.h>

#include <vsg/core/Mask.h>
#include <vsg/nodes/Node.h>
#include <vsg/ui/PointerEvent.h>

ObjectSelector::ObjectSelector(EditorContext& context)
    : context_(context)
{
}

void ObjectSelector::apply(vsg::ButtonPressEvent& buttonPress)
{
    if (buttonPress.handled)
    {
        return;
    }

    if (buttonPress.button != editor2::MOUSE_BUTTON_LEFT)
    {
        return;
    }

    // Клик по окну ImGui не должен выделять объекты сцены
    if (ImGui::GetCurrentContext() != nullptr &&
        ImGui::GetIO().WantCaptureMouse)
    {
        return;
    }

    if (context_.state != EditorState::EDIT_ROUTE)
    {
        return;
    }

    const LSIntersectorRefPtr intersector =
        context_.intersection_handler->get_lmb_intersector();

    if (!intersector)
    {
        return;
    }

    context_.scenegraph->accept(*intersector);

    auto& intersections = intersector->intersections;
    if (intersections.empty())
    {
        // Клик по пустому месту без Shift — снять выделение
        const RouteObjects& selected_objects = context_.selected_objects;

        if (!selected_objects.empty() && !context_.shift_pressed)
        {
            SelectObjects* const select_objects_command =
                new SelectObjects(context_);

            select_objects_command->objects_to_deselect = selected_objects;
            select_objects_command->update_description();

            context_.commands.push(select_objects_command, true);
        }

        return;
    }

    const LSIntersectionRefPtr intersection =
        IntersectionHandler::get_closest_intersection(intersector);

    if (!intersection)
    {
        return;
    }

    for (const vsg::Node* const node : intersection->nodePath)
    {
        if (const RouteObject* const object = node->cast<RouteObject>())
        {
            select_object(vsg::ref_ptr(const_cast<RouteObject*>(object)));
            break;
        }
    }

    intersections.clear();
}

void ObjectSelector::select_object(vsg::ref_ptr<RouteObject> object)
{
    SelectObjects* const select_objects_command =
        new SelectObjects(context_);

    RouteObjects& objects_to_select =
        select_objects_command->objects_to_select;

    RouteObjects& objects_to_deselect =
        select_objects_command->objects_to_deselect;

    const bool shift_pressed = context_.shift_pressed;

    if (shift_pressed)
    {
        if (object->get_is_selected())
        {
            objects_to_deselect.emplace_back(object);
        }
        else
        {
            objects_to_select.emplace_back(object);
        }
    }
    else
    {
        const RouteObjects& selected_objects = context_.selected_objects;

        if (selected_objects.empty())
        {
            objects_to_select.emplace_back(object);
        }
        else if (object->get_is_selected())
        {
            if (selected_objects.size() == 1)
            {
                objects_to_deselect.emplace_back(object);
            }
            else
            {
                for (const auto& selected_object : selected_objects)
                {
                    if (selected_object != object)
                    {
                        objects_to_deselect.emplace_back(selected_object);
                    }
                }
            }
        }
        else
        {
            objects_to_select.emplace_back(object);
            objects_to_deselect = selected_objects;
        }
    }

    select_objects_command->update_description();

    context_.commands.push(select_objects_command, true);
}
