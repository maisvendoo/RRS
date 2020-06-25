#include    "qviewer.h"

#include    "qt-events-handler.h"
#include    "quit-handler.h"
#include    "esc-press-handler.h"

#include    "menu.h"

#include    <osgWidget/WindowManager>
#include    <osgWidget/ViewerEventHandlers>

#include    <QDir>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
QViewer::QViewer(QObject *parent) : QObject(parent), osgViewer::Viewer()
  , camera(new osg::Camera)
  , settings(settings_t())
  , wm(nullptr)
  , root_dir("")
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

    this->setKeyEventSetsDone(0);

    return osgViewer::Viewer::run();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void QViewer::init(const settings_t &settings,
                   const command_line_t &cmd_line,
                   QString root_dir)
{
    this->root_dir = root_dir;

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

    osg::ref_ptr<MainMenu> menu = new MainMenu;

    menu->init(QDir::toNativeSeparators(root_dir) + QDir::separator() + "cfg" + QDir::separator() + "gui.xml",
               QDir::toNativeSeparators(root_dir) + QDir::separator() + "fonts");

    menu->addItem("Выход");
    osg::ref_ptr<QuitHandler> quit_handler = new QuitHandler;
    connect(menu->getItem(0), &Label::action, quit_handler, &QuitHandler::quit);
    this->addEventHandler(quit_handler.get());

    menu->addItem("Назад");
    osg::ref_ptr<EscPressHandler> esc_handler = new EscPressHandler(menu.get(), wm.get());
    connect(menu->getItem(1), &Label::action, esc_handler, &EscPressHandler::slotHide);
    this->addEventHandler(esc_handler.get());    

    wm->addChild(menu.get());
    wm->resizeAllWindows();    

    osg::ref_ptr<osg::Group> root = new osg::Group;

    osg::Camera *cam = wm->createParentOrthoCamera();

    root->addChild(cam);

    menu->hide();


    addEventHandler(new osgWidget::MouseHandler(wm.get()) );
    addEventHandler(new osgWidget::KeyboardHandler(wm.get()) );
    addEventHandler(new osgWidget::ResizeHandler(wm.get(), cam) );
    addEventHandler(new osgWidget::CameraSwitchHandler(wm.get(), cam) );

    this->setSceneData(root.get());
}


