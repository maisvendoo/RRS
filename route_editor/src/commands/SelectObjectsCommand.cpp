#include "SelectObjectsCommand.h"

#include "Gizmo.h"
#include "RouteObject.h"

#include <cstdio>
#include <string>
#include <utility>

Gizmo* SelectObjectsCommand::s_gizmo = nullptr;

SelectObjectsCommand::SelectObjectsCommand(
    const RouteObjects& objects_to_select,
    const RouteObjects& objects_to_deselect
)
    : objects_to_select(objects_to_select)
    , objects_to_deselect(objects_to_deselect)
{
}

SelectObjectsCommand::SelectObjectsCommand(
    const RouteObjects& objects_to_select,
    const RouteObjects&& objects_to_deselect
)
    : objects_to_select(objects_to_select)
    , objects_to_deselect(std::move(objects_to_deselect))
{
}

SelectObjectsCommand::SelectObjectsCommand(
    const RouteObjects&& objects_to_select,
    const RouteObjects& objects_to_deselect
)
    : objects_to_select(std::move(objects_to_select))
    , objects_to_deselect(objects_to_deselect)
{
}

SelectObjectsCommand::SelectObjectsCommand(
    const RouteObjects&& objects_to_select,
    const RouteObjects&& objects_to_deselect
)
    : objects_to_select(std::move(objects_to_select))
    , objects_to_deselect(std::move(objects_to_deselect))
{
}

void SelectObjectsCommand::execute() const
{
    for (const auto& object : objects_to_select)
    {
        object->select();
    }

    for (const auto& object : objects_to_deselect)
    {
        object->deselect();
    }

    s_gizmo->update_visibility();
}

void SelectObjectsCommand::undo() const
{
    for (const auto& object : objects_to_select)
    {
        object->deselect();
    }

    for (const auto& object : objects_to_deselect)
    {
        object->select();
    }

    s_gizmo->update_visibility();
}

std::string SelectObjectsCommand::to_string() const
{
    char buffer[128];
    std::snprintf(buffer, 128,
        "Select objects: to select: %zu objects\n"
        "                to deselect: %zu objects",
        objects_to_select.size(), objects_to_deselect.size());
    return buffer;
}

void SelectObjectsCommand::set_gizmo(Gizmo* gizmo)
{
    s_gizmo = gizmo;
}
