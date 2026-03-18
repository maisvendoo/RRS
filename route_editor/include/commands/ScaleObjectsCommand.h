#ifndef SCALE_OBJECTS_COMMAND_H
#define SCALE_OBJECTS_COMMAND_H

#include "Command.h"
#include "RouteObjects.h"

#include <vsg/maths/mat4.h>
#include <vsg/maths/vec3.h>

#include <vector>

struct EditorContext;

class ScaleObjectsCommand : public Command
{
public:
    ScaleObjectsCommand(EditorContext& context, vsg::vec3 pivot,
        vsg::vec3 scale);

    virtual ~ScaleObjectsCommand() override = default;
    virtual void execute() const override;
    virtual void undo() const override;
    virtual void update_description() override;

private:
    const RouteObjects objects;
    std::vector<vsg::dmat4> initial_matrices;
    vsg::vec3 pivot;
    vsg::vec3 scale;
};

#endif // SCALE_OBJECTS_COMMAND_H
