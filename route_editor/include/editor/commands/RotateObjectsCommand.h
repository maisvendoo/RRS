#ifndef EDITOR_COMMANDS_ROTATE_OBJECTS_COMMAND_H
#define EDITOR_COMMANDS_ROTATE_OBJECTS_COMMAND_H

#include "editor/commands/TransformObjectsCommand.h"

#include <vsg/maths/vec3.h>

struct EditorContext;

class RotateObjectsCommand : public TransformObjectsCommand
{
public:
    RotateObjectsCommand(EditorContext& context, const RouteObjects& objects,
        const vsg::dvec3& pivot, const vsg::dvec3& axis, double radians);

    virtual ~RotateObjectsCommand() override = default;

    virtual void execute() override;

    virtual void update_description() override;

private:
    vsg::dvec3 pivot_;
    vsg::dvec3 axis_;
    double radians_;
};

#endif // EDITOR_COMMANDS_ROTATE_OBJECTS_COMMAND_H
