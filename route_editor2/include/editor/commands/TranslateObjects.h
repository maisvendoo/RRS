#ifndef EDITOR_TRANSLATE_OBJECTS_H
#define EDITOR_TRANSLATE_OBJECTS_H

#include "editor/commands/TransformObjects.h"

#include <vsg/maths/vec3.h>

struct EditorContext;

/// Перемещение объектов
class TranslateObjects : public TransformObjects
{
public:
    TranslateObjects(
        EditorContext& context,
        const RouteObjects& objects,
        const vsg::dvec3& translation
    );

    virtual void execute() override;
    virtual void update_description() override;

private:
    vsg::dvec3 translation_;
};

#endif // EDITOR_TRANSLATE_OBJECTS_H
