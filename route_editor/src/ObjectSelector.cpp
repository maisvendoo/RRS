#include "editor/ObjectSelector.h"

#include "editor/EditorContext.h"
#include "editor/Gizmo.h"

#include <vsg/ui/PointerEvent.h>

ObjectSelector::ObjectSelector(EditorContext& context)
    : editor_context(context)
{
}

void ObjectSelector::apply(vsg::ButtonReleaseEvent& buttonRelease)
{
    editor_context.gizmo->apply(buttonRelease);
}

void ObjectSelector::apply(vsg::MoveEvent& moveEvent)
{
    editor_context.gizmo->apply(moveEvent);
}
