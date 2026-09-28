#include "editor/commands/Command.h"

Command::Command(EditorContext& editor_context)
    : editor_context(editor_context)
{
}

Command::~Command() = default;

const char* Command::get_description() const
{
    return description;
}
