//------------------------------------------------------------------------------
//
//      Video client's window manager
//      (c) maisvendoo
//
//------------------------------------------------------------------------------
/*!
 * \file
 * \brief Video client's window manager
 * \copyright maisvendoo
 * \author maisvendoo
 * \date
 */

#include    "viewer.h"

#include    <osg/BlendFunc>
#include    <osg/CullFace>
#include    <osg/GraphicsContext>
#include    <osgDB/FileUtils>
#include    <osgDB/FileNameUtils>
#include    <osgViewer/ViewerEventHandlers>
#include    <osg/LightModel>
#include    <osgViewer/View>

#include    "filesystem.h"
#include    "config-reader.h"

#include    <sstream>
#include    <fstream>

#include    "notify.h"
#include    "abstract-loader.h"
#include    "lighting.h"
#include    "motion-blur.h"
#include    "qt-events.h"
#include    "switch-view.h"
#include    "screen-capture.h"
#include    "hud.h"

#include    "rails-manipulator.h"

#include    <QObject>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
RouteViewer::RouteViewer(int argc, char *argv[])
  : is_ready(false)
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
    // Qt signals processing
    viewer.addEventHandler(new QtEventsHandler());

    // Keyboard events handler
    keyboard = new KeyboardHandler();
    viewer.addEventHandler(keyboard);

    // Camera switch handler
    //viewer.addEventHandler(new CameraViewHandler());

    //client.init(settings, &viewer);

    QObject::connect(keyboard, &KeyboardHandler::sendKeyBoardState,
                     &client, &NetworkClient::receiveKeysState);

    viewer.addEventHandler(new osgViewer::StatsHandler);
    //viewer.setThreadingModel(osgViewer::Viewer::SingleThreaded);

    osg::ref_ptr<RailsManipulator> rm = new RailsManipulator;
    QObject::connect(train_ext_handler, &TrainExteriorHandler::sendTrainPosition,
                     rm, &RailsManipulator::getTrainPosition);

    viewer.setCameraManipulator(rm.get());

    return viewer.run();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool RouteViewer::init(int argc, char *argv[])
{
    FileSystem &fs = FileSystem::getInstance();

    // Read settings from config file
    settings = loadSettings(fs.getConfigDir() + fs.separator() + "settings.xml");

    // Parse command line
    CommandLineParser parser(argc, argv);
    cmd_line_t cmd_line = parser.getCommadLine();
    overrideSettingsByCommandLine(cmd_line, settings);

    // Notify settings
    osg::NotifySeverity level = osg::INFO;

    if (settings.notify_level == "INFO")
        level = osg::INFO;

    if (settings.notify_level == "WARN")
        level = osg::WARN;

    if (settings.notify_level == "FATAL")
        level = osg::FATAL;

    osg::setNotifyLevel(level);
    std::string logs_path = fs.getLogsDir();
    osg::setNotifyHandler(new LogFileHandler(logs_path + fs.separator() + "viewer.log"));

    // Load selected route
    if (!loadRoute(cmd_line.route_dir.value))
    {
        OSG_FATAL << "Route from " << cmd_line.route_dir.value << " is't loaded" << std::endl;
        return false;
    }

    // Init graphical engine settings
    if (!initEngineSettings(root.get()))
        return false;

    // Init display settings
    if (!initDisplay(&viewer, settings))
        return false;

    // Init motion blur
    if (!initMotionBlurEffect(&viewer, settings))
        return false;

    osg::ref_ptr<osgViewer::ScreenCaptureHandler::CaptureOperation> writeFile =
            new WriteToFileOperation(fs.getScreenshotsDir());

    osg::ref_ptr<osgViewer::ScreenCaptureHandler> screenCaptureHandler =
            new osgViewer::ScreenCaptureHandler(writeFile.get());

    viewer.addEventHandler(screenCaptureHandler.get());
    viewer.addEventHandler(new ScreenCaptureHandler(screenCaptureHandler.get()));

    HUD *hud = new HUD(settings.width, settings.height);
    root->addChild(hud->getCamera());

    viewer.addEventHandler(new KeyboardHUDHandler(hud->getScene()));

    QObject::connect(train_ext_handler, &TrainExteriorHandler::setStatusBar,
                     hud, &HUD::setStatusBar);

    osgDB::DatabasePager *dp = viewer.getDatabasePager();
    dp->setDoPreCompile(true);
    dp->setTargetMaximumNumberOfPageLOD(1000);

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
        cfg.getValue(secName, "VSync", settings.vsync);
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
        cfg.getValue(secName, "RequestInterval", settings.request_interval);
        cfg.getValue(secName, "ReconnectInterval", settings.reconnect_interval);
        cfg.getValue(secName, "MotionBlur", settings.persistence);
        cfg.getValue(secName, "NotifyLevel", settings.notify_level);
        cfg.getValue(secName, "ViewDistance", settings.view_distance);
    }

    return settings;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void RouteViewer::overrideSettingsByCommandLine(const cmd_line_t &cmd_line,
                                                settings_t &settings)
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

    if (cmd_line.train_config.is_present)
        settings.train_config = cmd_line.train_config.value;

    if (cmd_line.notify_level.is_present)
        settings.notify_level = cmd_line.notify_level.value;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool RouteViewer::loadRoute(const std::string &routeDir)
{
    if (routeDir.empty())
    {
        OSG_FATAL << "ERROR: Route path is empty" << std::endl;
        OSG_FATAL << "Route path: " << routeDir << std::endl;
        return false;
    }

    FileSystem &fs = FileSystem::getInstance();
    std::string routeType = osgDB::findDataFile(routeDir + fs.separator() + "route-type");

    if (routeType.empty())
    {
        OSG_FATAL << "ERROR: File route-type is not found in route directory" << std::endl;
        return false;
    }

    std::ifstream stream(routeType);

    if (!stream.is_open())
    {
        OSG_FATAL << "ERROR: Stream for route-type file is't open" << std::endl;
        return false;
    }

    std::string routeExt = "";
    stream >> routeExt;

    if (routeExt.empty())
    {
        OSG_FATAL << "ERROR: Unknown route type" << std::endl;
        return false;
    }

    std::string routeLoaderPlugin = routeExt + "-route-loader";

    osg::ref_ptr<RouteLoader> loader = loadRouteLoader(fs.getPluginsDir(), routeLoaderPlugin);

    if (!loader.valid())
    {
        OSG_FATAL << "ERROR: Not found route loader for this route" << std::endl;
        return false;
    }

    loader->load(routeDir, settings.view_distance);

    MotionPath *motionPath = loader->getMotionPath(settings.direction);

    train_ext_handler = new TrainExteriorHandler(motionPath, settings.train_config);
    viewer.addEventHandler(train_ext_handler);
    viewer.addEventHandler(train_ext_handler->getAnimationManager());

    root = new osg::Group;
    root->addChild(train_ext_handler->getExterior());
    root->addChild(loader->getRoot());

    return true;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool RouteViewer::initEngineSettings(osg::Group *root)
{
    if (root == nullptr)
        return false;

    // Common graphics settings
    osg::StateSet *stateset = root->getOrCreateStateSet();

    stateset->setMode(GL_DEPTH_TEST, osg::StateAttribute::ON);

    stateset->setMode(GL_LIGHTING, osg::StateAttribute::ON);
    stateset->setMode(GL_NORMALIZE, osg::StateAttribute::ON);

    osg::ref_ptr<osg::CullFace> cull = new osg::CullFace;
    cull->setMode(osg::CullFace::BACK);
    stateset->setAttributeAndModes(cull.get(), osg::StateAttribute::ON);

    // Set lighting
    initEnvironmentLight(root,
                         osg::Vec4(1.0f, 1.0f, 1.0f, 1.0f),
                         1.0f,
                         -20.0f,
                         75.0f);

    osg::LightModel *lightmodel = new osg::LightModel;
    float power = 0.4f;
    lightmodel->setAmbientIntensity(osg::Vec4(power, power, power, 1.0));
    lightmodel->setTwoSided(true);
    stateset->setAttributeAndModes(lightmodel, osg::StateAttribute::ON);

    return true;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool RouteViewer::initDisplay(osgViewer::Viewer *viewer,
                              const settings_t &settings)
{
    if (viewer == nullptr)
        return false;

    viewer->setSceneData(root.get());

    osg::ref_ptr<osg::GraphicsContext::Traits> traits = new osg::GraphicsContext::Traits;
    traits->x = settings.x;
    traits->y = settings.y;
    traits->width = settings.width;
    traits->height = settings.height;
    traits->windowName = settings.name;
    traits->windowDecoration = settings.window_decoration;
    traits->doubleBuffer = settings.double_buffer;
    traits->samples = settings.samples;
    traits->vsync = settings.vsync;

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
    {
        (*it)->add(new MotionBlurOperation(settings.persistence));        
    }

    return true;
}
