#ifndef EDITOR_RESET_SCALE_H
#define EDITOR_RESET_SCALE_H

#include "editor/RouteObject.h"
#include "editor/commands/Command.h"

#include <vsg/maths/mat4.h>

#include <vector>

struct EditorContext;

/// Сброс масштаба объектов к (1, 1, 1) (окно «Mass edit»).
/// Конструктор запоминает исходные матрицы - undo возвращает их
class ResetScale : public Command
{
public:
    ResetScale(EditorContext& context, const RouteObjects& objects);

    virtual void execute() override;
    virtual void undo() override;
    virtual void update_description() override;

private:
    RouteObjects objects_;
    std::vector<vsg::dmat4> initial_matrices_;
};

#endif // EDITOR_RESET_SCALE_H
