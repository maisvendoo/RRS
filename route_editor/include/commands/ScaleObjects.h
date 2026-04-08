#ifndef SCALE_OBJECTS_H
#define SCALE_OBJECTS_H

#include "commands/TransformObjects.h"

#include <vsg/maths/vec3.h>

struct EditorContext;

class ScaleObjects : public TransformObjects
{
public:
    ScaleObjects(EditorContext& context, const vsg::dvec3& pivot,
        const vsg::dvec3& scale);

    virtual ~ScaleObjects() override = default;
    virtual void execute() override;
    virtual void update_description() override;

private:
    vsg::dvec3 pivot;
    vsg::dvec3 scale;
};

#endif // SCALE_OBJECTS_H
