#include    "display-loader.h"

#include    <osg/Notify>

#include    "display-visitor.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
AbstractDisplay *loadDisplayModule(const QString &module_path,
                                   const QString &surface_name,
                                   osg::Node *model)
{
    AbstractDisplay *display = loadDisplay(module_path);

    if (display == nullptr)
    {
        OSG_FATAL << "Module " << module_path.toStdString() << " is't found";
        return display;
    }

    DisplayVisitor dv(display, surface_name);
    dv.setTraversalMode(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN);
    model->accept(dv);

    return display;
}
