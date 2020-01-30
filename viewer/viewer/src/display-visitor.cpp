#include    "display-visitor.h"

#include    <osg/MatrixTransform>
#include    <osgQt/QWidgetImage>
#include    <osg/Texture2D>

#include    "display-surface-visitor.h"

DisplayVisitor::DisplayVisitor(display_container_t *dc, display_config_t display_config)
    : osg::NodeVisitor()
    , dc(dc)
    , display_config(display_config)
{

}

void DisplayVisitor::apply(osg::Transform &transform)
{
    osg::MatrixTransform *matrix_trans = static_cast<osg::MatrixTransform *>(&transform);

    if (matrix_trans->getName() == display_config.surface_name.toStdString())
    {
        DisplaySurfaceVisitor dsv(dc, display_config);
        dsv.setTraversalMode(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN);
        matrix_trans->accept(dsv);
    }

    traverse(transform);
}
