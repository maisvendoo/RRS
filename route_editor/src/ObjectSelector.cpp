#include "editor/ObjectSelector.h"

#include "editor/EditorContext.h"
#include "editor/Gizmo.h"

#include <vsg/ui/PointerEvent.h>

ObjectSelector::ObjectSelector(EditorContext& context)
    : editor_ctx(context)
{
}

void ObjectSelector::apply(vsg::ButtonReleaseEvent& buttonRelease)
{
    editor_ctx.gizmo->apply(buttonRelease);
}

void ObjectSelector::apply(vsg::MoveEvent& moveEvent)
{
    editor_ctx.gizmo->apply(moveEvent);
}
