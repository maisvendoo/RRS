#ifndef MOVE_OBJECTS_COMMAND_H
#define MOVE_OBJECTS_COMMAND_H

#include "commands/TransformObjectsCommand.h"

#include <vsg/maths/vec3.h>

struct EditorContext;

class TranslateObjects : public TransformObjectsCommand
{
public:
    TranslateObjects(EditorContext& context, vsg::vec3 translation);
    virtual ~TranslateObjects() override = default;
    virtual void execute() override;
    virtual void update_description() override;

private:
    vsg::vec3 translation;
};

#endif // MOVE_OBJECTS_COMMAND_H
