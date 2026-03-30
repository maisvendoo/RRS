#ifndef TRANSFORM_OBJECTS_COMMAND_H
#define TRANSFORM_OBJECTS_COMMAND_H

#include "Command.h"
#include "RouteObject.h"

#include <vsg/maths/mat4.h>

#include <vector>

struct EditorContext;

class TransformObjectsCommand : public Command
{
public:
    explicit TransformObjectsCommand(EditorContext& context);
    virtual ~TransformObjectsCommand() override = default;
    virtual void undo() override;

protected:
    const RouteObjects objects;

private:
    std::vector<vsg::dmat4> initial_matrices;
};

#endif // TRANSFORM_OBJECTS_COMMAND_H
