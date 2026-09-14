#ifndef EDITOR_TOGGLE_VISIBILITY_H
#define EDITOR_TOGGLE_VISIBILITY_H

#include "editor/RouteObject.h"
#include "editor/commands/Command.h"

#include <vector>

struct EditorContext;

/// Скрыть/показать выделенные объекты (маска узла MASK_ALL/MASK_OFF).
/// Объекты остаются в route1.map - скрытие только визуальное
/// (рабочий инструмент, промт п.8: скрытие мешей объектов)
class ToggleVisibility : public Command
{
public:
    explicit ToggleVisibility(EditorContext& context);

    virtual void execute() override;
    virtual void undo() override;
    virtual void update_description() override;

private:
    RouteObjects objects_;
    std::vector<vsg::Mask> initial_masks_;
};

#endif // EDITOR_TOGGLE_VISIBILITY_H
