#ifndef EDITOR_COMMANDS_SELECT_OBJECTS_COMMAND_H
#define EDITOR_COMMANDS_SELECT_OBJECTS_COMMAND_H

#include "editor/commands/Command.h"

#include <cstddef>
#include <vector>

struct EditorContext;
class Gizmo;
class StaticObjectManager;

class SelectObjectsCommand : public Command
{
public:
    std::vector<std::size_t> ids_to_select;
    std::vector<std::size_t> ids_to_deselect;

public:
    explicit SelectObjectsCommand(EditorContext& context);
    virtual ~SelectObjectsCommand() override = default;

    virtual void execute() override;
    virtual void undo() override;
    virtual void update_description() override;

private:
    StaticObjectManager& static_object_manager;
    Gizmo& gizmo;
};

#endif // EDITOR_COMMANDS_SELECT_OBJECTS_COMMAND_H
