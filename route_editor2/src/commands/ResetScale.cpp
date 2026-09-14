#include "editor/commands/ResetScale.h"

#include "editor/commands/Command.h"
#include "editor/EditorContext.h"
#include "editor/RouteObject.h"

#include <cstdio>

ResetScale::ResetScale(EditorContext& context, const RouteObjects& objects)
    : Command(context)
    , objects_(objects)
{
    // Исходные матрицы запоминаются до сброса (undo вернёт их)
    initial_matrices_.reserve(objects.size());

    for (const auto& object : objects_)
    {
        object->save_matrix();
        initial_matrices_.push_back(object->get_initial_matrix());
    }

    update_description();
}

void ResetScale::execute()
{
    for (const auto& object : objects_)
    {
        object->set_scale(vsg::dvec3{1.0, 1.0, 1.0});
    }

    context_.status = "Масштаб сброшен: " + std::to_string(objects_.size()) +
        " объектов";
}

void ResetScale::undo()
{
    std::size_t index = 0;

    for (const auto& object : objects_)
    {
        object->set_matrix(initial_matrices_[index]);
        ++index;
    }
}

void ResetScale::update_description()
{
    std::snprintf(description_, COMMAND_DESCRIPTION_BUFFER_SIZE,
        "Reset scale: %zu objects",
        objects_.size()
    );
}
