#ifndef EDITOR_DELETE_OBJECTS_H
#define EDITOR_DELETE_OBJECTS_H

#include "editor/RouteObject.h"
#include "editor/commands/Command.h"

#include <vector>

struct EditorContext;

/// Удаление выделенных объектов
class DeleteObjects : public Command
{
public:
    /// Удалить выделенные объекты
    explicit DeleteObjects(EditorContext& context);

    /// Удалить заданный список объектов (окно «Mass edit»:
    /// отфильтрованные объекты, не только выделенные)
    DeleteObjects(EditorContext& context, const RouteObjects& objects);

    virtual void execute() override;
    virtual void undo() override;
    virtual void update_description() override;

private:
    RouteObjects objects_;

    /// Было ли объектом выделение до удаления (восстановление
    /// выделения в undo)
    std::vector<bool> was_selected_;
};

#endif // EDITOR_DELETE_OBJECTS_H
