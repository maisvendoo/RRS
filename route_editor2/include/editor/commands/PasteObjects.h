#ifndef EDITOR_PASTE_OBJECTS_H
#define EDITOR_PASTE_OBJECTS_H

#include "editor/RouteObject.h"
#include "editor/commands/Command.h"

struct EditorContext;

/// Вставка скопированных объектов (со смещением от оригиналов).
/// Копии создаются RouteObject::copy(); undo удаляет их из сцены
class PasteObjects : public Command
{
public:
    explicit PasteObjects(EditorContext& context);

    virtual void execute() override;
    virtual void undo() override;
    virtual void update_description() override;

private:
    RouteObjects copied_objects_;
    vsg::dvec3 offset_ = {10.0, 10.0, 0.0};
};

#endif // EDITOR_PASTE_OBJECTS_H
