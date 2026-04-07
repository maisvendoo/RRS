// #ifndef TRANSFORM_SINGLE_OBJECT_H
// #define TRANSFORM_SINGLE_OBJECT_H

// #include "commands/Command.h"

// #include <vsg/core/ref_ptr.h>
// #include <vsg/maths/mat4.h>

// struct EditorContext;
// class RouteObject;

// class TransformSingleObjectCommand : public Command
// {
// public:
//     TransformSingleObjectCommand(
//         EditorContext& context,
//         vsg::ref_ptr<RouteObject> object
//     );

//     virtual ~TransformSingleObjectCommand() override = default;

//     virtual void undo() override;

// protected:
//     const vsg::ref_ptr<RouteObject> object;

// private:
//     vsg::dmat4 initial_matrix;
// };

// #endif // TRANSFORM_SINGLE_OBJECT_H
