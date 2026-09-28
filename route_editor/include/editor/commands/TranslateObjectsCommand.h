#ifndef EDITOR_COMMANDS_TRANSLATE_OBJECTS_COMMAND_H
#define EDITOR_COMMANDS_TRANSLATE_OBJECTS_COMMAND_H

#include "editor/commands/TransformObjectsCommand.h"

#include <vsg/maths/vec3.h>

struct EditorContext;

class TranslateObjectsCommand : public TransformObjectsCommand
{
public:
    TranslateObjectsCommand(EditorContext& context,
        const RouteObjects& objects, const vsg::dvec3& translation);

    virtual ~TranslateObjectsCommand() override = default;

    virtual void execute() override;

    virtual void update_description() override;

private:
    vsg::dvec3 translation_;
};

#endif // EDITOR_COMMANDS_TRANSLATE_OBJECTS_COMMAND_H
