#ifndef EDITOR_SCALE_OBJECTS_H
#define EDITOR_SCALE_OBJECTS_H

#include "editor/commands/TransformObjects.h"

#include <vsg/maths/vec3.h>

struct EditorContext;

/// Масштабирование объектов относительно точки
class ScaleObjects : public TransformObjects
{
public:
    ScaleObjects(
        EditorContext& context,
        const RouteObjects& objects,
        const vsg::dvec3& pivot,
        const vsg::dvec3& scale
    );

    virtual void execute() override;
    virtual void update_description() override;

private:
    vsg::dvec3 pivot_;
    vsg::dvec3 scale_;
};

#endif // EDITOR_SCALE_OBJECTS_H
