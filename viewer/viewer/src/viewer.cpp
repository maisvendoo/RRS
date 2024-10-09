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

#include    <viewer.h>

#include    <osg/BlendFunc>
#include    <osg/CullFace>
#include    <osg/GraphicsContext>
#include    <osgDB/FileUtils>
#include    <osgDB/FileNameUtils>
#include    <osgViewer/ViewerEventHandlers>
#include    <osg/LightModel>
#include    <osgViewer/View>

#include    <filesystem.h>
#include    <config-reader.h>

#include    <sstream>
#include    <fstream>

#include    <notify.h>
#include    <abstract-loader.h>
#include    <lighting.h>
#include    <qt-events.h>
#include    <screen-capture.h>
#include    <viewer-stats-switcher.h>
#include    <rails-manipulator.h>
#include    <free-manipulator.h>
#include    <stat-manipulator.h>
#include    <train-manipulator.h>
#include    <camera-switcher.h>

#include    <QObject>

#include    <imgui-widgets-handler.h>

#include    <QThread>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
RouteViewer::RouteViewer(int argc, char *argv[], QObject *parent) : QObject(parent)
{
    memory_sim_info.setKey(SHARED_MEMORY_SIM_INFO);

    if (memory_sim_info.attach(QSharedMemory::ReadOnly))
    {
        //OSG_FATAL << "Connected to shared memory with simulator info" << std::endl;
    }
    else
    {
        //OSG_FATAL << "Can't connect to shared memory with simulator info" << std::endl;
    }

    is_ready = init(argc, argv);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
RouteViewer::~RouteViewer()
{
    memory_sim_info.detach();
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
    viewer.addEventHandler(new QtEventsHandler(settings.interval));

    // Keyboard events handler
    keyboard = new KeyboardHandler();
    viewer.addEventHandler(keyboard);

    osg::ref_ptr<osgViewer::StatsHandler> statsHandler = new ViewerStatsHandler();
    statsHandler->setKeyEventTogglesOnScreenStats(osgGA::GUIEventAdapter::KEY_F11);

    viewer.addEventHandler(statsHandler.get());

    //viewer.setThreadingModel(osgViewer::Viewer::SingleThreaded);

    // Cabine camera manipulator
    osg::ref_ptr<RailsManipulator> rm = new RailsManipulator(settings);
    QObject::connect(train_ext_handler, &TrainExteriorHandler::sendCameraPosition,
                     rm, &RailsManipulator::getCameraPosition);


    // Free camera manipulator
    osg::ref_ptr<FreeManipulator> fm = new FreeManipulator(settings);
    QObject::connect(train_ext_handler, &TrainExteriorHandler::sendCameraPosition,
                     fm, &FreeManipulator::getCameraPosition);

    // Static camera manipulator
    osg::ref_ptr<StaticManipulator> sm_right = new StaticManipulator(settings, true);
    QObject::connect(train_ext_handler, &TrainExteriorHandler::sendCameraPosition,
                     sm_right, &StaticManipulator::getCameraPosition);

    // Static camera manipulator
    osg::ref_ptr<StaticManipulator> sm_left = new StaticManipulator(settings, false);
    QObject::connect(train_ext_handler, &TrainExteriorHandler::sendCameraPosition,
                     sm_left, &StaticManipulator::getCameraPosition);

    // Static camera manipulator
    osg::ref_ptr<TrainManipulator> tm = new TrainManipulator(settings);
    QObject::connect(train_ext_handler, &TrainExteriorHandler::sendCameraPosition,
                     tm, &TrainManipulator::getCameraPosition);

    osg::ref_ptr<CameraSwitcher> cs = new CameraSwitcher;
    cs->addMatrixManipulator(osgGA::GUIEventAdapter::KEY_F2, "cabine_view", rm.get());
    cs->addMatrixManipulator(osgGA::GUIEventAdapter::KEY_F3, "train_view", tm.get());
    cs->addMatrixManipulator(osgGA::GUIEventAdapter::KEY_F4, "free_view", fm.get());
    cs->addMatrixManipulator(osgGA::GUIEventAdapter::KEY_F5, "static_view", sm_right.get());
    cs->addMatrixManipulator(osgGA::GUIEventAdapter::KEY_F6, "static_view", sm_left.get());

    viewer.setCameraManipulator(cs.get());
    viewer.addEventHandler(cs.get());

    viewer.setKeyEventSetsDone(0);
    viewer.setRealizeOperation(new ImGuiInitOperation);

    // Обработка интерфейса ImGUI
    osg::ref_ptr<ImGuiWidgetsHandler> imguiWidgetsHandler = new ImGuiWidgetsHandler;

    QObject::connect(train_ext_handler, &TrainExteriorHandler::setStatusBar,
                         imguiWidgetsHandler.get(), &ImGuiWidgetsHandler::setStatusBar);

    QObject::connect(train_ext_handler, &TrainExteriorHandler::sendControlledState,
                     imguiWidgetsHandler.get(), &ImGuiWidgetsHandler::receiveControlledState);


    viewer.addEventHandler(imguiWidgetsHandler.get());

    // Добляем обработчик событий сигналов
    viewer.addEventHandler(traffic_lights_handler.get());

    // Инициализация TCP-клиента
    initTCPclient(settings);

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
    osg::setNotifyHandler(new ViewerLogFileHandler(logs_path + fs.separator() + "viewer.log"));

    OSG_FATAL << "Override settings from command line" << std::endl;
    // Parse command line
    CommandLineParser parser(argc, argv);
    cmd_line_t cmd_line = parser.getCommadLine();
    overrideSettingsByCommandLine(cmd_line, settings);

    OSG_FATAL << "Override settings from simulator shared memory" << std::endl;
    // Parse info from shared memory
    overrideSettingsBySharedMemory(settings);

    try
    {
        sound_manager = new SoundManager();
        OSG_FATAL << "Created SoundManager" << std::endl;
    }
    catch (const std::bad_alloc &)
    {
        OSG_FATAL << "SoundManager is not created";
    }

    // Load selected route
    if (!loadRoute())
    {
        OSG_FATAL << "Route from " << settings.route_dir_name << " is't loaded" << std::endl;
        return false;
    }

    // Init graphical engine settings
    if (!initEngineSettings(root.get()))
        return false;

    // Init display settings
    if (!initDisplay(&viewer, settings))
        return false;

    // Init motion blur
    /*if (!initMotionBlurEffect(&viewer, settings))
        return false;*/

    // Запись скриншота в файл
    osg::ref_ptr<osgViewer::ScreenCaptureHandler::CaptureOperation> writeFile =
            new WriteToFileOperation(fs.getScreenshotsDir());

    osg::ref_ptr<osgViewer::ScreenCaptureHandler> screenCaptureHandler =
            new ScreenCapture(writeFile.get());

    // Одиночный скриншот по клавише F12
    screenCaptureHandler->setKeyEventTakeScreenShot(osgGA::GUIEventAdapter::KEY_F12);
    // Серия скриншотов отключена из-за просадки производительности
    screenCaptureHandler->setKeyEventToggleContinuousCapture(-1);

    viewer.addEventHandler(screenCaptureHandler.get());        

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

        cfg.getValue(secName, "CabineCamRotCoeff", settings.cabine_cam_rot_coeff);
        cfg.getValue(secName, "CabineCamFovYStep", settings.cabine_cam_fovy_step);
        cfg.getValue(secName, "CabineCamSpeed", settings.cabine_cam_speed);

        cfg.getValue(secName, "ExtCamInitDist", settings.ext_cam_init_dist);
        cfg.getValue(secName, "ExtCamInitHeight", settings.ext_cam_init_height);
        cfg.getValue(secName, "ExtCamInitShift", settings.ext_cam_init_shift);
        cfg.getValue(secName, "ExtCamRotCoeff", settings.ext_cam_rot_coeff);
        cfg.getValue(secName, "ExtCamSpeed", settings.ext_cam_speed);
        cfg.getValue(secName, "ExtCamSpeedCoeff", settings.ext_cam_speed_coeff);
        cfg.getValue(secName, "ExtCamMinDist", settings.ext_cam_min_dist);
        cfg.getValue(secName, "ExtCamInitAngleH", settings.ext_cam_init_angle_H);
        cfg.getValue(secName, "ExtCamInitAngleV", settings.ext_cam_init_angle_V);

        std::string tmp = "";
        cfg.getValue(secName, "FreeCamInitPos", tmp);
        std::istringstream ss(tmp);

        ss >> settings.free_cam_init_pos.x()
           >> settings.free_cam_init_pos.y()
           >> settings.free_cam_init_pos.z();

        cfg.getValue(secName, "FreeCamRotCoeff", settings.free_cam_rot_coeff);
        cfg.getValue(secName, "FreeCamSpeed", settings.free_cam_speed);
        cfg.getValue(secName, "FreeCamSpeedCoeff", settings.free_cam_speed_coeff);
        cfg.getValue(secName, "FreeCamFovY", settings.free_cam_fovy_step);

        cfg.getValue(secName, "StatCamDist", settings.stat_cam_dist);
        cfg.getValue(secName, "StatCamHeight", settings.stat_cam_height);
        cfg.getValue(secName, "StatCamShift", settings.stat_cam_shift);

        cfg.getValue(secName, "FrameDiv", settings.interval);
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

    //if (cmd_line.train_config.is_present)
    //    settings.train_config = cmd_line.train_config.value;

    if (cmd_line.notify_level.is_present)
        settings.notify_level = cmd_line.notify_level.value;

    if (cmd_line.direction.is_present)
        settings.direction = cmd_line.direction.value;

    if (cmd_line.route_dir.is_present)
        settings.route_dir_name = cmd_line.route_dir.value;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void RouteViewer::overrideSettingsBySharedMemory(settings_t &settings)
{
    if (memory_sim_info.lock())
    {
        simulator_info_t *tmp = static_cast<simulator_info_t *>(memory_sim_info.data());
        if (tmp == nullptr)
        {
            memory_sim_info.unlock();
            OSG_FATAL << "ERROR: shared memory is null" << std::endl;
            return;
        }
        if (tmp->num_updates <= 0)
        {
            memory_sim_info.unlock();
            OSG_FATAL << "ERROR: shared memory isn't updated with sim info." << std::endl;
            OSG_FATAL << "Try to wait for 10 seconds." << std::endl;
            for (int i = 0; i < 10; ++i)
            {
                QThread::sleep(1000);
                if (memory_sim_info.lock())
                {
                    if (tmp->num_updates <= 0)
                    {
                        OSG_FATAL << "ERROR: shared memory isn't updated with sim info (after " << i + 1 << "seconds)." << std::endl;
                        memory_sim_info.unlock();
                        continue;
                    }
                    else
                    {
                        break;
                    }
                }
            }
        }
        if (tmp->num_updates <= 0)
        {
            OSG_FATAL << "ERROR: shared memory isn't updated with sim info." << std::endl;
            memory_sim_info.unlock();
            return;
        }

        memcpy(&info_data, tmp, sizeof (simulator_info_t));
        memory_sim_info.unlock();
        OSG_FATAL << "Got simulator info from shared memory" << std::endl;

        QString route_dir_tmp = QString::fromStdWString(info_data.route_info.route_dir_name);
        route_dir_tmp.resize(info_data.route_info.route_dir_name_length);
        settings.route_dir_name = route_dir_tmp.toStdString();
        OSG_FATAL << "Route directory name from shared memory: " << route_dir_tmp.toStdString() << std::endl;
    }
    else
    {
        OSG_FATAL << "ERROR: Can't lock shared memory" << std::endl;
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool RouteViewer::loadRoute()
{
    if (settings.route_dir_name.empty())
    {
        OSG_FATAL << "ERROR: Route directory name is empty" << std::endl;
        return false;
    }

    FileSystem &fs = FileSystem::getInstance();
    std::string route_dir_path = fs.combinePath(fs.getRouteRootDir(), settings.route_dir_name);
    settings.route_dir_full_path = route_dir_path;
    std::string routeType = osgDB::findDataFile(route_dir_path + fs.separator() + "route-type");

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

    loader->load(route_dir_path, settings.view_distance);

    //MotionPath *motionPath = loader->getMotionPath(settings.direction);

    train_ext_handler = new TrainExteriorHandler(settings, sound_manager, info_data);
    viewer.addEventHandler(train_ext_handler);

    //viewer.addEventHandler(train_ext_handler->getAnimationManager());
    std::vector<AnimationManager *> anims_manager = train_ext_handler->getAnimManagers();

    for (auto am : anims_manager)
    {
        viewer.addEventHandler(am);
    }

    root = new osg::Group;
    root->addChild(train_ext_handler->getExterior());
    root->addChild(loader->getRoot());

    // Грузим модельки сигналов
    traffic_lights_handler->create_pagedLODs(settings);
    root->addChild(traffic_lights_handler->getSignalsGroup());    

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

    camera-> setComputeNearFarMode (osg :: CullSettings :: DO_NOT_COMPUTE_NEAR_FAR);
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
void RouteViewer::initTCPclient(const settings_t &settings)
{
    tcp_config_t tcp_config;
    tcp_config.host_addr = QString(settings.host_addr.c_str());
    tcp_config.port = settings.port;
    tcp_config.reconnect_interval = settings.reconnect_interval;
    tcp_config.request_interval = settings.request_interval;

    connect(tcp_client, &TcpClient::connected,
            this, &RouteViewer::slotConnectedToSimulator);

    connect(tcp_client, &TcpClient::setSignalsData,
            this, &RouteViewer::slotGetSignalsData);

    tcp_client->init(tcp_config);

    OSG_FATAL << "Client is initilized...OK\n";
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void RouteViewer::slotConnectedToSimulator()
{
    OSG_FATAL << "Connected to server...OK\n";

    tcp_client->sendRequest(STYPE_SIGNALS_LIST);    
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void RouteViewer::slotGetSignalsData(QByteArray &sig_data)
{
    traffic_lights_handler->deserialize(sig_data);

    traffic_lights_handler->load_signal_models(settings);

    for (auto am : traffic_lights_handler->animation_mangers)
    {
        viewer.addEventHandler(am);
    }
}
