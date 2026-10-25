#ifndef OBJECT_SELECTOR_H
#define OBJECT_SELECTOR_H

#include <vsg/core/Inherit.h>
#include <vsg/core/Visitor.h>

struct EditorContext;

namespace vsg
{

class ButtonReleaseEvent;
class MoveEvent;

} // namespace vsg

class ObjectSelector : public vsg::Inherit<vsg::Visitor, ObjectSelector>
{
public:
    ObjectSelector(EditorContext& editor_ctx);

    void apply(vsg::ButtonReleaseEvent& buttonRelease) override;
    void apply(vsg::MoveEvent& moveEvent) override;

private:
    EditorContext& editor_ctx;
};

#endif // OBJECT_SELECTOR_H
