#ifndef EDITOR_OBJECT_SELECTOR_H
#define EDITOR_OBJECT_SELECTOR_H

#include <vsg/core/Inherit.h>
#include <vsg/core/Visitor.h>
#include <vsg/core/ref_ptr.h>

struct EditorContext;
class RouteObject;

namespace vsg
{

class ButtonPressEvent;

}

/// Выделение объектов кликом ЛКМ (Shift — мультивыделение)
class ObjectSelector : public vsg::Inherit<vsg::Visitor, ObjectSelector>
{
public:
    explicit ObjectSelector(EditorContext& context);

    void apply(vsg::ButtonPressEvent& buttonPress) override;

private:
    void select_object(vsg::ref_ptr<RouteObject> object);

private:
    EditorContext& context_;
};

#endif // EDITOR_OBJECT_SELECTOR_H
