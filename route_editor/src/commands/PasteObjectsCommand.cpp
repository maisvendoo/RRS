#include "PasteObjectsCommand.h"

#include "Command.h"
#include "EditorContext.h"
#include "Route.h"
#include "RouteObject.h"

#include <vsg/app/Viewer.h>
#include <vsg/core/Mask.h>

PasteObjectsCommand::PasteObjectsCommand(EditorContext& context)
    : Command(context)
    , objects_to_paste(context.copied_objects)
    , objects_to_deselect(context.selected_objects)
{
}

void PasteObjectsCommand::execute()
{
    for (RouteObject* const object : objects_to_paste)
    {
        const auto new_object = object->copy();
        context.route->addChild(vsg::MASK_ALL, new_object);
        context.objects.emplace_back(new_object);
        pasted_objects.emplace_back(new_object);

        if (object->get_is_selected())
        {
            object->deselect();
        }

        new_object->select();
    }

    const auto compile_result = context.viewer->compileManager->compile(
        context.route);

    vsg::updateViewer(*context.viewer, compile_result);
}

void PasteObjectsCommand::undo()
{

}

void PasteObjectsCommand::update_description()
{

}
