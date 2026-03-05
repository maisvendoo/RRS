#include "HideObjectsCommand.h"

#include "RouteObject.h"

#include <cstdio>
#include <string>

HideObjectsCommand::HideObjectsCommand(
    EditorContext& context,
    const RouteObjects& objects_to_hide,
    const RouteObjects& objects_to_show
)
    : Command(context)
    , objects_to_hide(objects_to_hide)
    , objects_to_show(objects_to_show)
{
}

HideObjectsCommand::HideObjectsCommand(
    EditorContext& context,
    const RouteObjects& objects_to_hide,
    const RouteObjects&& objects_to_show
)
    : Command(context)
    , objects_to_hide(objects_to_hide)
    , objects_to_show(std::move(objects_to_show))
{
}

HideObjectsCommand::HideObjectsCommand(
    EditorContext& context,
    const RouteObjects&& objects_to_hide,
    const RouteObjects& objects_to_show
)
    : Command(context)
    , objects_to_hide(std::move(objects_to_hide))
    , objects_to_show(objects_to_show)
{
}

HideObjectsCommand::HideObjectsCommand(
    EditorContext& context,
    const RouteObjects&& objects_to_hide,
    const RouteObjects&& objects_to_show
)
    : Command(context)
    , objects_to_hide(std::move(objects_to_hide))
    , objects_to_show(std::move(objects_to_show))
{
}

void HideObjectsCommand::execute() const
{
    for (const auto& object : objects_to_hide)
    {
        object->hide();
    }

    for (const auto& object : objects_to_show)
    {
        object->show();
    }
}

void HideObjectsCommand::undo() const
{
    for (const auto& object : objects_to_hide)
    {
        object->show();
    }

    for (const auto& object : objects_to_show)
    {
        object->hide();
    }
}

std::string HideObjectsCommand::to_string() const
{
    char buffer[128];
    std::snprintf(buffer, 128,
        "Hide objects: to hide: %zu objects\n"
        "              to show: %zu objects",
        objects_to_hide.size(), objects_to_show.size());
    return buffer;
}
