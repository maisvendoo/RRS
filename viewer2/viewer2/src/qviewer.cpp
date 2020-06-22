#include    "qviewer.h"

#include    "qt-events-handler.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
QViewer::QViewer(QObject *parent) : QObject(parent), osgViewer::Viewer()
  , camera(new osg::Camera)
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
QViewer::~QViewer()
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
int QViewer::run()
{
    osg::ref_ptr<QtEventsHandler> qt_events_hendler = new QtEventsHandler;
    this->addEventHandler(qt_events_hendler.get());

    return osgViewer::Viewer::run();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void QViewer::init(const settings_t &settings,
                   const command_line_t &cmd_line)
{
    initWindow(settings);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void QViewer::initWindow(const settings_t &settings)
{
    osg::ref_ptr<osg::GraphicsContext::Traits> traits = new osg::GraphicsContext::Traits;
    traits->x = settings.posX;
    traits->y = settings.posY;
    traits->width = settings.width;
    traits->height = settings.height;
    traits->windowName = "RRS viewer";
    traits->windowDecoration = settings.window_title;
    traits->doubleBuffer = settings.double_buffer;
    traits->samples = settings.samples;

    osg::ref_ptr<osg::GraphicsContext> gc = osg::GraphicsContext::createGraphicsContext(traits.get());
    camera->setGraphicsContext(gc.get());
    camera->setViewport(new osg::Viewport(0, 0, traits->width, traits->height));

    if (traits->height == 0)
        traits->height = 1;

    double aspect = static_cast<double>(traits->width) / static_cast<double>(traits->height);
    camera->setProjectionMatrixAsPerspective(settings.fovY, aspect, settings.zNear, settings.zFar);

    if (settings.fullscreen)
        setUpViewOnSingleScreen(settings.screen_num);
    else
        setUpViewInWindow(traits->x, traits->y, traits->width, traits->height);
}
