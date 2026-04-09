#ifndef MOVE_OBJECTS_H
#define MOVE_OBJECTS_H

#include "commands/TransformObjects.h"

#include <vsg/maths/vec3.h>

struct EditorContext;

class TranslateObjects : public TransformObjects
{
public:
    TranslateObjects(EditorContext& context, const vsg::dvec3& translation);
    virtual ~TranslateObjects() override = default;
    virtual void execute() override;
    virtual void update_description() override;

private:
    vsg::dvec3 translation_;
};

#endif // MOVE_OBJECTS_H
