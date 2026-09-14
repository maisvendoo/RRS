#ifndef EDITOR_TRANSFORM_OBJECTS_H
#define EDITOR_TRANSFORM_OBJECTS_H

#include "editor/RouteObject.h"
#include "editor/commands/Command.h"

#include <vsg/maths/mat4.h>

#include <vector>

struct EditorContext;

/// Базовая команда трансформации: запоминает исходные матрицы объектов
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

#endif // EDITOR_TRANSFORM_OBJECTS_H
