#ifndef TRANSFORM_OBJECTS_H
#define TRANSFORM_OBJECTS_H

#include "RouteObjects.h"
#include "commands/Command.h"

#include <vsg/maths/mat4.h>

#include <vector>

struct EditorContext;

class TransformObjects : public Command
{
public:
    TransformObjects(EditorContext& context, const RouteObjects& objects);

    virtual ~TransformObjects() override = default;

    virtual void undo() override;

protected:
    const RouteObjects objects_;

private:
    std::vector<vsg::dmat4> initial_matrices_;
};

#endif // TRANSFORM_OBJECTS_H
