// #include    "display-visitor.h"

// #include    <osg/MatrixTransform>
// #include    <osg/Texture2D>

// #include    "display-surface-visitor.h"

// DisplayVisitor::DisplayVisitor(display_container_t *dc, display_config_t display_config)
//     : osg::NodeVisitor()
//     , dc(dc)
//     , display_config(display_config)
// {

// }

// void DisplayVisitor::apply(osg::Transform &transform)
// {
//     osg::MatrixTransform *matrix_trans = static_cast<osg::MatrixTransform *>(&transform);

//     if (matrix_trans->getName() == display_config.surface_name.toStdString())
//     {
//         OSG_INFO << "Founded node for display with name: " << matrix_trans->getName() << std::endl;
//         DisplaySurfaceVisitor dsv(dc, display_config);
//         dsv.setTraversalMode(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN);
//         matrix_trans->accept(dsv);
//     }

//     traverse(transform);
// }

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

#include "display-visitor.h"

#include "display-config.h"
#include "Logger.h"
#include "display-surface-visitor.h"

#include <vsg/core/Visitor.h>
#include <vsg/nodes/MatrixTransform.h>

DisplayVisitor::DisplayVisitor(display_container_t* dc, const display_config_t& display_config)
    : dc(dc)
    , display_config(display_config)
{
}

void DisplayVisitor::apply(vsg::Node& transform)
{
    std::string name;
    transform.getValue("name", name);
    if (name.empty())
    {
        transform.getValue("name", name);
    }
    if (name.empty() || name != display_config.surface_name.toStdString())
    {
        transform.traverse(*this);
        return;
    }

    LOG_INFO("Found node for display with name: %s", name.c_str());

    DisplaySurfaceVisitor dsv(dc, display_config);
    transform.accept(dsv);

    transform.traverse(*this);
}
