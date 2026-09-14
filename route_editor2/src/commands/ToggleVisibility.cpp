#include "editor/commands/ToggleVisibility.h"

#include "editor/commands/Command.h"
#include "editor/EditorContext.h"

#include <vsg/core/Mask.h>

#include <cstdio>

ToggleVisibility::ToggleVisibility(EditorContext& context)
    : Command(context)
    , objects_(context.selected_objects)
{
    for (const auto& object : objects_)
    {
        initial_masks_.push_back(object->mask);
    }

    update_description();
}

void ToggleVisibility::execute()
{
    // Переключение: видимые скрываются, скрытые показываются
    for (const auto& object : objects_)
    {
        object->mask = (object->mask == vsg::MASK_OFF)
                ? vsg::MASK_ALL : vsg::MASK_OFF;
        object->deselect();
    }

    context_.selected_objects.clear();
}

void ToggleVisibility::undo()
{
    std::size_t i = 0;
    for (const auto& object : objects_)
    {
        object->mask = initial_masks_[i];
        object->select();
        ++i;
    }

    context_.selected_objects = objects_;
}

void ToggleVisibility::update_description()
{
    std::snprintf(description_, COMMAND_DESCRIPTION_BUFFER_SIZE,
        "Toggle visibility: %zu objects",
        objects_.size()
    );
}
