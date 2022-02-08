#include    "display-loader.h"

#include    <osg/Notify>

#include    "display.h"
#include    "display-visitor.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void loadDisplayModule(const display_config_t &display_config,
                                       display_container_t *dc,
                                       osg::Node *model)
{
    dc->display = loadDisplay(display_config.module_name);

    if (dc->display == nullptr)
    {
        OSG_FATAL << "Module " << display_config.module_name.toStdString() << " is't found";
        return;
    }    

    DisplayVisitor dv(dc, display_config);
    dv.setTraversalMode(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN);
    model->accept(dv);
}
