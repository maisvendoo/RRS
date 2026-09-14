#ifndef EDITOR_PASTE_PREFAB_H
#define EDITOR_PASTE_PREFAB_H

#include "editor/RouteObject.h"
#include "editor/commands/Command.h"

#include <vsg/maths/vec3.h>

struct EditorContext;

/// Вставка префаба из окна «Префабы»: копии объектов (созданы
/// RouteObject::copy() до команды) переносятся к точке перед
/// камерой общим смещением - взаимное расположение объектов
/// префаба сохраняется. Undo удаляет копии из сцены
/// (по образцу PasteObjects)
class PastePrefab : public Command
{
public:
    PastePrefab(EditorContext& context, RouteObjects copied_objects,
        const vsg::dvec3& offset, const std::string& prefab_name);

    virtual void execute() override;
    virtual void undo() override;
    virtual void update_description() override;

private:
    RouteObjects copied_objects_;
    vsg::dvec3 offset_ = {0.0, 0.0, 0.0};
    std::string prefab_name_;
};

#endif // EDITOR_PASTE_PREFAB_H
