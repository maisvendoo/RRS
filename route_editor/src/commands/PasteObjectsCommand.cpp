#include "PasteObjectsCommand.h"

#include "Command.h"
#include "EditorContext.h"

PasteObjectsCommand::PasteObjectsCommand(EditorContext& context)
    : Command(context)
    , objects_to_paste(context.copied_objects)
    , objects_to_deselect(context.selected_objects)
{
}

void PasteObjectsCommand::execute()
{

}

void PasteObjectsCommand::undo()
{

}

void PasteObjectsCommand::update_description()
{

}
