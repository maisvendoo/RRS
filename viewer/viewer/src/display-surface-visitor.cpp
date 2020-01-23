#include    "display-surface-visitor.h"

#include    <osgQt/QWidgetImage>
#include    <osgViewer/ViewerEventHandlers>
#include    <osg/Material>

DisplaySurfaceVisitor::DisplaySurfaceVisitor(osgQt::QWidgetImage *widgetImage)
    : osg::NodeVisitor()
    , widgetImage(widgetImage)

{

}

void DisplaySurfaceVisitor::apply(osg::Geode &geode)
{
    osgViewer::InteractiveImageHandler *handler = new osgViewer::InteractiveImageHandler(widgetImage);
    geode.setEventCallback(handler);
    geode.setCullCallback(handler);

    for (unsigned int i = 0; i < geode.getNumDrawables(); ++i)
    {
        /*osg::Drawable *drawable = geode.getDrawable(i);
        osg::ref_ptr<osg::StateSet> stateset = new osg::StateSet(*drawable->getOrCreateStateSet(), osg::CopyOp::DEEP_COPY_STATESETS);
        osg::ref_ptr<osg::Material> mat = new osg::Material;

        mat->setDiffuse(osg::Material::FRONT_AND_BACK, osg::Vec4(1.0f, 1.0f, 1.0f, 1.0f));
        mat->setAmbient(osg::Material::FRONT_AND_BACK, osg::Vec4(1.0f, 1.0f, 1.0f, 1.0f));

        stateset->setAttribute(mat.get());
        drawable->setStateSet(stateset.get());

        int zu = 0;*/
    }

    traverse(geode);
}
