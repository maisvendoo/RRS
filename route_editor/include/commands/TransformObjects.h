#ifndef TRANSFORM_OBJECTS_H
#define TRANSFORM_OBJECTS_H

#include "commands/Command.h"
#include "RouteObject.h"

#include <vsg/maths/mat4.h>

#include <vector>

struct EditorContext;

class TransformObjects : public Command
{
public:
    explicit TransformObjects(EditorContext& context);
    virtual ~TransformObjects() override = default;
    virtual void undo() override;

protected:
    const RouteObjects objects;

private:
    std::vector<vsg::dmat4> initial_matrices;
};

#endif // TRANSFORM_OBJECTS_H
