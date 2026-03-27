#ifndef PASTE_OBJECTS_COMMAND_H
#define PASTE_OBJECTS_COMMAND_H

#include "Command.h"
#include "RouteObjects.h"

class PasteObjectsCommand : Command
{
public:
    explicit PasteObjectsCommand(EditorContext& context);
    virtual ~PasteObjectsCommand() = default;
    virtual void execute() override;
    virtual void undo() override;
    virtual void update_description() override;

private:
    RouteObjects objects_to_paste;
    RouteObjects pasted_objects;
    RouteObjects objects_to_deselect;
};

// Копирование
// Создаем новые объекты на основе старых,
// добавляем их к Route,
// делаем updateViewer,
// снимаем выделение с уже выбранных объектов,
// выделяем новые

// Отмена
// Снимаем выделение с новых объектов,
// выделяем старые,
// удаляем модели из Route и вообще,
// делаем updateViewer

#endif // PASTE_OBJECTS_COMMAND_H
