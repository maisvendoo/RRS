#ifndef SCALE_OBJECTS_COMMAND_H
#define SCALE_OBJECTS_COMMAND_H

#include "TransformObjectsCommand.h"

#include <vsg/maths/vec3.h>

struct EditorContext;

class ScaleObjectsCommand : public TransformObjectsCommand
{
public:
    ScaleObjectsCommand(EditorContext& context, vsg::vec3 pivot,
        vsg::vec3 scale);

    virtual ~ScaleObjectsCommand() override = default;
    virtual void execute() const override;
    virtual void update_description() override;

private:
    vsg::vec3 pivot;
    vsg::vec3 scale;
};

#endif // SCALE_OBJECTS_COMMAND_H
