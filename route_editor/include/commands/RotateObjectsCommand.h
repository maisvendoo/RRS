#ifndef ROTATE_OBJECTS_COMMAND_H
#define ROTATE_OBJECTS_COMMAND_H

#include "TransformObjectsCommand.h"

#include <vsg/maths/vec3.h>

struct EditorContext;

class RotateObjectsCommand : public TransformObjectsCommand
{
public:
    RotateObjectsCommand(EditorContext& context,vsg::vec3 pivot,
        vsg::vec3 axis, float radians);

    virtual ~RotateObjectsCommand() override = default;
    virtual void execute() override;
    virtual void update_description() override;

private:
    vsg::vec3 pivot;
    vsg::vec3 axis;
    float radians;
};

#endif // ROTATE_OBJECTS_COMMAND_H
