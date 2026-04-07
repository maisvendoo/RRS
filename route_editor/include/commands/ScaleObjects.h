#ifndef SCALE_OBJECTS_H
#define SCALE_OBJECTS_H

#include "commands/TransformObjects.h"

#include <vsg/maths/vec3.h>

struct EditorContext;

class ScaleObjects : public TransformObjects
{
public:
    ScaleObjects(EditorContext& context, vsg::vec3 pivot,
        vsg::vec3 scale);

    virtual ~ScaleObjects() override = default;
    virtual void execute() override;
    virtual void update_description() override;

private:
    vsg::vec3 pivot;
    vsg::vec3 scale;
};

#endif // SCALE_OBJECTS_H
