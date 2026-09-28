#ifndef EDITOR_COMMANDS_TRANSFORM_OBJECTS_COMMAND_H
#define EDITOR_COMMANDS_TRANSFORM_OBJECTS_COMMAND_H

#include "editor/RouteObjects.h"
#include "editor/commands/Command.h"

#include <vsg/maths/mat4.h>

#include <vector>

struct EditorContext;

class TransformObjectsCommand : public Command
{
public:
    TransformObjectsCommand(EditorContext& context, const RouteObjects& objects);

    virtual ~TransformObjectsCommand() override = default;

    virtual void undo() override;

protected:
    const RouteObjects objects_;

private:
    std::vector<vsg::dmat4> initial_matrices_;
};

#endif // EDITOR_COMMANDS_TRANSFORM_OBJECTS_COMMAND_H
