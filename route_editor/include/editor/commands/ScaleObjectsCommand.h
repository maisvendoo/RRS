#ifndef EDITOR_COMMANDS_SCALE_OBJECTS_COMMAND_H
#define EDITOR_COMMANDS_SCALE_OBJECTS_COMMAND_H

#include "editor/commands/TransformObjectsCommand.h"

#include <vsg/maths/vec3.h>

struct EditorContext;

class ScaleObjectsCommand : public TransformObjectsCommand
{
public:
    ScaleObjectsCommand(EditorContext& context, const RouteObjects& objects,
        const vsg::dvec3& pivot, const vsg::dvec3& scale);

    virtual ~ScaleObjectsCommand() override = default;

    virtual void execute() override;

    virtual void update_description() override;

private:
    vsg::dvec3 pivot_;
    vsg::dvec3 scale_;
};

#endif // EDITOR_COMMANDS_SCALE_OBJECTS_COMMAND_H
