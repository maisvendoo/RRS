#include    "display-visitor.h"

#include    <osg/MatrixTransform>
#include    <osgQt/QWidgetImage>
#include    <osg/Texture2D>

#include    "display-surface-visitor.h"

DisplayVisitor::DisplayVisitor(AbstractDisplay *display, QString surfaceName)
    : osg::NodeVisitor()
    , display(display)
    , surfaceName(surfaceName)
{

}

void DisplayVisitor::apply(osg::Transform &transform)
{
    osg::MatrixTransform *matrix_trans = static_cast<osg::MatrixTransform *>(&transform);

    if (matrix_trans->getName() == surfaceName.toStdString())
    {
        osg::ref_ptr<osgQt::QWidgetImage> widgetImage = new osgQt::QWidgetImage(display);

        osg::ref_ptr<osg::Texture2D> texture = new osg::Texture2D(widgetImage.get());

        texture->setResizeNonPowerOfTwoHint(false);
        texture->setFilter(osg::Texture::MIN_FILTER, osg::Texture::LINEAR);
        texture->setWrap(osg::Texture::WRAP_S, osg::Texture::CLAMP_TO_EDGE);
        texture->setWrap(osg::Texture::WRAP_T, osg::Texture::CLAMP_TO_EDGE);

        osg::StateSet *stateset = matrix_trans->getOrCreateStateSet();
        stateset->setTextureAttributeAndModes(0, texture, osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);

        DisplaySurfaceVisitor dsv(widgetImage.get());
        dsv.setTraversalMode(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN);
        matrix_trans->accept(dsv);
    }

    traverse(transform);
}
