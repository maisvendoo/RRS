#include "editor/commands/ReplaceLabel.h"

#include "editor/commands/Command.h"
#include "editor/EditorContext.h"
#include "editor/RouteObject.h"

#include <vsg/nodes/PagedLOD.h>

#include <algorithm>
#include <cstddef>
#include <cstdio>

ReplaceLabel::ReplaceLabel(EditorContext& context,
    const RouteObjects& objects,
    const std::string& new_label)
    : Command(context)
    , objects_(objects)
    , new_label_(new_label)
{
    old_labels_.reserve(objects.size());
    old_lods_.reserve(objects.size());

    for (const auto& object : objects_)
    {
        old_labels_.push_back(object->label);
        old_lods_.push_back(object->get_paged_lod());
    }

    update_description();
}

void ReplaceLabel::execute()
{
    const auto ref_it = context_.objects_ref.find(new_label_);

    const bool has_model = ref_it != context_.objects_ref.end() &&
        ref_it->second.paged_lod != nullptr;

    models_swapped_ = has_model;

    for (const auto& object : objects_)
    {
        move_route_map_entry(context_, object->label, new_label_,
            object->get_translation());

        object->label = new_label_;

        if (has_model)
        {
            object->replace_model(ref_it->second.paged_lod);
        }
    }

    if (has_model)
    {
        context_.status = "Метка заменена: " + new_label_;
    }
    else
    {
        context_.status = "Метка заменена: " + new_label_ +
            " (модель пересоздастся при перезагрузке)";
    }
}

void ReplaceLabel::undo()
{
    std::size_t i = 0;
    for (const auto& object : objects_)
    {
        move_route_map_entry(context_, new_label_, old_labels_[i],
            object->get_translation());

        if (models_swapped_ && old_lods_[i])
        {
            object->replace_model(old_lods_[i]);
        }

        object->label = old_labels_[i];
        ++i;
    }
}

void ReplaceLabel::update_description()
{
    std::snprintf(description_, COMMAND_DESCRIPTION_BUFFER_SIZE,
        "Replace label -> \"%s\": %zu objects",
        new_label_.c_str(),
        objects_.size()
    );
}

void ReplaceLabel::move_route_map_entry(EditorContext& context,
    const std::string& from_label, const std::string& to_label,
    const vsg::dvec3& translation)
{
    if (from_label == to_label)
    {
        return;
    }

    const auto from_it = context.route_map.find(from_label);

    if (from_it == context.route_map.end())
    {
        return;
    }

    auto& transforms = from_it->second;

    const auto transform_it = std::find_if(transforms.begin(),
        transforms.end(),
        [&translation](const RouteMapTransformation& transform) -> bool {
            return vsg::length(transform.translation - translation) < 1.0e-6;
        });

    if (transform_it == transforms.end())
    {
        return;
    }

    context.route_map[to_label].push_back(*transform_it);
    transforms.erase(transform_it);

    if (transforms.empty())
    {
        context.route_map.erase(from_it);
    }
}
