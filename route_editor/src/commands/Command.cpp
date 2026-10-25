#include "editor/commands/Command.h"

Command::Command(EditorContext& editor_ctx)
    : editor_ctx(editor_ctx)
{
}

Command::~Command() = default;

const char* Command::get_description() const
{
    return description;
}
