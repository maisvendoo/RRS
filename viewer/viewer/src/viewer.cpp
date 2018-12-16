#include    "viewer.h"

#include    <osg/BlendFunc>
#include    <osg/CullFace>
#include    <osg/GraphicsContext>
#include    <osgDB/ReadFile>

#include    "filesystem.h"
#include    "config-reader.h"

#include    <sstream>
#include    <fstream>

#include    "abstract-loader.h"
#include    "lighting.h"
#include    "motion-blur.h"
#include    "route-loading-handle.h"
#include    "log.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
RouteViewer::RouteViewer(int argc, char *argv[])
  : current_route_id(0),
    is_ready(false)
{
    is_ready = init(argc, argv);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
RouteViewer::~RouteViewer()
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool RouteViewer::isReady() const
{
    return is_ready;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
int RouteViewer::run()
{
    viewer.addEventHandler(new QtEventsHandler());
    viewer.addEventHandler(new RouteLoadingHandle());

    client.init(settings, &viewer);    

    return viewer.run();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool RouteViewer::init(int argc, char *argv[])
{
    FileSystem &fs = FileSystem::getInstance();    

    // Notify configure
    osg::setNotifyLevel(osg::WARN);
    osg::setNotifyHandler(new LogFileHandler(fs.getLogsDir() + fs.separator() + "startup.log"));

    // Read settings from config file
    settings = loadSettings(fs.getConfigDir() + fs.separator() + "settings.xml");

    // Parse command line
    CommandLineParser parser(argc, argv);
    cmd_line_t cmd_line = parser.getCommadLine();
    overrideSettingsByCommandLine(cmd_line, settings);            

    // Init display settings
    if (!initDisplay(&viewer, settings))
        return false;

    // Init motion blur
    if (!initMotionBlurEffect(&viewer, settings))
        return false;

    return true;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
settings_t RouteViewer::loadSettings(const std::string &cfg_path) const
{
    settings_t settings;

    ConfigReader cfg(cfg_path);

    if (cfg.isOpenned())
    {
        std::string secName = "Viewer";

        cfg.getValue(secName, "HostAddress", settings.host_addr);
        cfg.getValue(secName, "Port", settings.port);
        cfg.getValue(secName, "Width", settings.width);
        cfg.getValue(secName, "Height", settings.height);
        cfg.getValue(secName, "FullScreen", settings.fullscreen);
        cfg.getValue(secName, "LocalMode", settings.localmode);
        cfg.getValue(secName, "posX", settings.x);
        cfg.getValue(secName, "posY", settings.y);
        cfg.getValue(secName, "FovY", settings.fovy);
        cfg.getValue(secName, "zNear", settings.zNear);
        cfg.getValue(secName, "zFar", settings.zFar);
        cfg.getValue(secName, "ScreenNumber", settings.screen_number);
        cfg.getValue(secName, "WindowDecoration", settings.window_decoration);
        cfg.getValue(secName, "DoubleBuffer", settings.double_buffer);
        cfg.getValue(secName, "Samples", settings.samples);
        cfg.getValue(secName, "Name", settings.name);
        cfg.getValue(secName, "RequestInterval", settings.request_interval);
        cfg.getValue(secName, "MotionBlur", settings.mb_persistence);
        cfg.getValue(secName, "ReconnectInterval", settings.reconnect_interval);
    }

    return settings;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void RouteViewer::overrideSettingsByCommandLine(const cmd_line_t &cmd_line,
                                                settings_t settings)
{
    if (cmd_line.host_addr.is_present)
        settings.host_addr = cmd_line.host_addr.value;

    if (cmd_line.port.is_present)
        settings.port = cmd_line.port.value;

    if (cmd_line.width.is_present)
        settings.width = cmd_line.width.value;

    if (cmd_line.height.is_present)
        settings.height = cmd_line.height.value;

    if (cmd_line.fullscreen.is_present)
        settings.fullscreen = cmd_line.fullscreen.value;

    if (cmd_line.localmode.is_present)
        settings.localmode = cmd_line.localmode.value;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool RouteViewer::initDisplay(osgViewer::Viewer *viewer,
                              const settings_t &settings)
{
    if (viewer == nullptr)
        return false;

    osg::ref_ptr<osg::GraphicsContext::Traits> traits = new osg::GraphicsContext::Traits;
    traits->x = settings.x;
    traits->y = settings.y;
    traits->width = settings.width;
    traits->height = settings.height;
    traits->windowName = settings.name;
    traits->windowDecoration = settings.window_decoration;
    traits->doubleBuffer = settings.double_buffer;
    traits->samples = settings.samples;

    osg::ref_ptr<osg::GraphicsContext> gc = osg::GraphicsContext::createGraphicsContext(traits.get());
    osg::Camera *camera = viewer->getCamera();

    camera->setGraphicsContext(gc.get());
    camera->setViewport(new osg::Viewport(0, 0, traits->width, traits->height));

    camera->setClearColor(osg::Vec4(0.63f, 0.80f, 0.97f, 1.0f));
    camera->setClearMask(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    double aspect = static_cast<double>(traits->width) / static_cast<double>(traits->height);
    camera->setProjectionMatrixAsPerspective(settings.fovy, aspect, settings.zNear, settings.zFar);

    camera->setAllowEventFocus(false);

    if (settings.fullscreen)
        viewer->setUpViewOnSingleScreen(settings.screen_number);

    return true;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool RouteViewer::initMotionBlurEffect(osgViewer::Viewer *viewer,
                                       const settings_t &settings)
{
    (void) settings;

    if (viewer == nullptr)
        return false;

    osg::DisplaySettings::instance()->setMinimumNumAccumBits(8, 8, 8, 8);
    viewer->realize();

    osgViewer::Viewer::Windows windows;
    viewer->getWindows(windows);

    for (auto it = windows.begin(); it != windows.end(); ++it)
        (*it)->add(new MotionBlurOperation(settings.mb_persistence));

    return true;
}
