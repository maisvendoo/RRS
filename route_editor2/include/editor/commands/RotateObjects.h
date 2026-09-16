#ifndef EDITOR_ROTATE_OBJECTS_H
#define EDITOR_ROTATE_OBJECTS_H

#include "editor/commands/TransformObjects.h"

#include <vsg/maths/vec3.h>

struct EditorContext;

/// Вращение объектов вокруг точки
class RotateObjects : public TransformObjects
{
public:
    RotateObjects(
        EditorContext& context,
        const RouteObjects& objects,
        const vsg::dvec3& pivot,
        const vsg::dvec3& axis,
        double radians
    );

    virtual void execute() override;
    virtual void update_description() override;

private:
    vsg::dvec3 pivot_;
    vsg::dvec3 axis_;
    double radians_;
};

#endif // EDITOR_ROTATE_OBJECTS_H
