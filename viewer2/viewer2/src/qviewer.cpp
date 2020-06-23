#include    "qviewer.h"

#include    "qt-events-handler.h"

#include    "menu.h"

#include    <osgWidget/WindowManager>
#include    <osgWidget/ViewerEventHandlers>

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

    initMainMenu();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void QViewer::initWindow(const settings_t &settings)
{
    this->settings = settings;

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

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void QViewer::initMainMenu()
{
    osg::ref_ptr<osgWidget::WindowManager> wm = new osgWidget::WindowManager(this,
                                                                settings.width,
                                                                settings.height,
                                                                0xF0000000);

    osg::ref_ptr<osgWidget::Window> menu = new osgWidget::Box();
    osgWidget::Label *label = new osgWidget::Label;
    label->setFontSize(14);
    label->setAlignHorizontal(osgWidget::Label::HA_CENTER);

    int width = 200;
    int height = 36;

    label->addWidth(width);
    label->addHeight(height);
    label->setColor(0.0, 0.0, 0.0, 1.0);
    label->setLabel(L"Return");
    menu->addWidget(label);

    menu->setX((settings.width - width) / 2);
    menu->setY((settings.height - height) / 2);

    wm->addChild(menu.get());
    wm->resizeAllWindows();

    menu->getBackground()->setColor(1.0f, 1.0f, 1.0f, 1.0f);
    //menu->resizePercent(100.0f);

    osg::ref_ptr<osg::Group> root = new osg::Group;

    osg::Camera *cam = wm->createParentOrthoCamera();

    root->addChild(cam);

    addEventHandler(new osgWidget::MouseHandler(wm.get()) );
    addEventHandler(new osgWidget::KeyboardHandler(wm.get()) );
    addEventHandler(new osgWidget::ResizeHandler(wm.get(), cam) );
    addEventHandler(new osgWidget::CameraSwitchHandler(wm.get(), cam) );

    this->setSceneData(root.get());
}
