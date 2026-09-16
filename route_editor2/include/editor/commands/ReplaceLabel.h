#ifndef EDITOR_REPLACE_LABEL_H
#define EDITOR_REPLACE_LABEL_H

#include "editor/RouteObject.h"
#include "editor/commands/Command.h"

#include <vsg/maths/vec3.h>
#include <vsg/nodes/PagedLOD.h>

#include <string>
#include <vector>

struct EditorContext;

/// Массовая замена метки объектов (окно «Mass edit», промт п.34-35):
/// меняет label, при наличии модели новой метки в objects.ref
/// подменяет и PagedLOD узла (модель меняется сразу),
/// иначе - только label (модель пересоздастся при перезагрузке).
/// Список route_map тоже переносится на новую метку
class ReplaceLabel : public Command
{
public:
    ReplaceLabel(EditorContext& context, const RouteObjects& objects,
        const std::string& new_label);

    virtual void execute() override;
    virtual void undo() override;
    virtual void update_description() override;

private:
    /// Перенос одной трансформации между метками route_map
    /// (объект опознаётся по позиции)
    static void move_route_map_entry(EditorContext& context,
        const std::string& from_label, const std::string& to_label,
        const vsg::dvec3& translation);

    RouteObjects objects_;
    std::vector<std::string> old_labels_;
    std::vector<vsg::ref_ptr<vsg::PagedLOD>> old_lods_;
    std::string new_label_;
    bool models_swapped_ = false;
};

#endif // EDITOR_REPLACE_LABEL_H
