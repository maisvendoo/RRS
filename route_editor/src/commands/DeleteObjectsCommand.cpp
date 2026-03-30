#include "DeleteObjectsCommand.h"

#include "Command.h"
#include "EditorContext.h"

DeleteObjectsCommand::DeleteObjectsCommand(EditorContext& context)
    : Command(context)
    , objects(context.selected_objects)
{
    update_description();
}

void DeleteObjectsCommand::execute()
{

}

void DeleteObjectsCommand::undo()
{

}

void DeleteObjectsCommand::update_description()
{

}
