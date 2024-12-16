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
#include    <osg/Texture>

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

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
RouteViewer::RouteViewer(int argc, char *argv[], QObject *parent) : QObject(parent)
{/*
    memory_sim_info.setKey(SHARED_MEMORY_SIM_INFO);

    if (memory_sim_info.attach(QSharedMemory::ReadOnly))
    {
        //OSG_FATAL << "Connected to shared memory with simulator info" << std::endl;
    }
    else
    {
        //OSG_FATAL << "Can't connect to shared memory with simulator info" << std::endl;
    }*/

    if (init(argc, argv))
    {
        OSG_INFO << "Viewer is initialized succesfully" << std::endl;
        std::cout << "Viewer is initialized succesfully" << std::endl;
        is_ready = true;
    }
    else
    {
        OSG_FATAL << "Fail to initialize viewer" << std::endl;
        std::cout << "Fail to initialize viewer" << std::endl;
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
RouteViewer::~RouteViewer()
{
    //memory_sim_info.detach();
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
    QObject::connect(keyboard, &KeyboardHandler::sendKeyBoardState,
                     this, &RouteViewer::slotUpdateKeyboard);

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


    return viewer.run();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool RouteViewer::init(int argc, char *argv[])
{
    FileSystem &fs = FileSystem::getInstance();

    osgDB::DatabasePager *dp = viewer.getDatabasePager();
    dp->setDoPreCompile(true);
    dp->setTargetMaximumNumberOfPageLOD(1000);

    // Read settings from config file
    settings = loadSettings(fs.getConfigDir() + fs.separator() + "settings.xml");

    std::cout << "Loaded settings from settings.xml" << std::endl;

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

    try
    {
        sound_manager = new SoundManager();
        OSG_FATAL << "Created SoundManager" << std::endl;
    }
    catch (const std::bad_alloc &)
    {
        OSG_FATAL << "SoundManager is not created";
    }

    train_ext_handler = new TrainExteriorHandler(settings, sound_manager);

    // Корневой узел сцены
    root = new osg::Group;

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

    // Инициализация TCP-клиента
    initTCPclient(settings);
/*
    OSG_FATAL << "Override settings from simulator shared memory" << std::endl;
    // Parse info from shared memory
    std::cout << "Try override settings from shared memory" << std::endl;
    //overrideSettingsBySharedMemory(settings);
    std::cout << "Overrided settings from shared memory" << std::endl;
*/
/*    // Load selected route
    if (!loadRoute())
    {
        OSG_FATAL << "Route from " << settings.route_dir_name << " is't loaded" << std::endl;
        std::cout << "Route from " << settings.route_dir_name << " is't loaded" << std::endl;
        return false;
    }
*/
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
        std::string secName = "Client";

        std::string tmp = "";
        if (cfg.getValue(secName, "HostAddr", tmp))
            settings.tcp_config.host_addr = tmp.c_str();
        int port = 0;
        if (cfg.getValue(secName, "port", port))
            settings.tcp_config.port = static_cast<quint16>(port);
        OSG_INFO << "Host for client from setings: " << tmp << ":" << port << std::endl;
        std::cout << "Host for client from setings: " << tmp << ":" << port << std::endl;
        cfg.getValue(secName, "ReconnectInteval", settings.tcp_config.reconnect_interval);
        cfg.getValue(secName, "VehiclesPosUpdateInterval", settings.vehicles_pos_update_interval);
        cfg.getValue(secName, "VehiclesStateUpdateInterval", settings.vehicles_state_update_interval);
        cfg.getValue(secName, "VehicleControlledUpdateInterval", settings.vehicle_controled_update_interval);

        secName = "Viewer";

        cfg.getValue(secName, "Width", settings.width);
        cfg.getValue(secName, "Height", settings.height);
        cfg.getValue(secName, "FullScreen", settings.fullscreen);
        cfg.getValue(secName, "VSync", settings.vsync);
        cfg.getValue(secName, "posX", settings.x);
        cfg.getValue(secName, "posY", settings.y);
        cfg.getValue(secName, "FovY", settings.fovy);
        cfg.getValue(secName, "zNear", settings.zNear);
        cfg.getValue(secName, "zFar", settings.zFar);
        cfg.getValue(secName, "ScreenNumber", settings.screen_number);
        cfg.getValue(secName, "WindowDecoration", settings.window_decoration);
        cfg.getValue(secName, "DoubleBuffer", settings.double_buffer);
        cfg.getValue(secName, "Samples", settings.samples);
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

        if (cfg.getValue(secName, "FreeCamInitPos", tmp))
        {
            std::istringstream ss(tmp);
            ss  >> settings.free_cam_init_pos.x()
                >> settings.free_cam_init_pos.y()
                >> settings.free_cam_init_pos.z();
        }

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
        settings.tcp_config.host_addr = QString::fromStdString(cmd_line.host_addr.value);

    if (cmd_line.port.is_present)
        settings.tcp_config.port = static_cast<quint16>(cmd_line.port.value);

    if (cmd_line.width.is_present)
        settings.width = cmd_line.width.value;

    if (cmd_line.height.is_present)
        settings.height = cmd_line.height.value;

    if (cmd_line.fullscreen.is_present)
        settings.fullscreen = cmd_line.fullscreen.value;

    if (cmd_line.notify_level.is_present)
        settings.notify_level = cmd_line.notify_level.value;

    if (cmd_line.direction.is_present)
        settings.direction = cmd_line.direction.value;

    if (cmd_line.route_dir.is_present)
        settings.route_dir_name = cmd_line.route_dir.value;
}
/*
//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void RouteViewer::overrideSettingsBySharedMemory(settings_t &settings)
{
    unsigned int delta = 10;
    unsigned int timeout = 10000;
    unsigned int time = 0;

    simulator_info_t *tmp = nullptr;

    while (time <= timeout)
    {
        tmp = static_cast<simulator_info_t *>(memory_sim_info.data());

        if (tmp != nullptr)
        {
            break;
        }

        QThread::sleep(delta);
        time += delta;
    }

    if (tmp == nullptr)
    {
        memory_sim_info.unlock();
        OSG_FATAL << "ERROR: shared memory is null" << std::endl;
        std::cout << "ERROR: shared memory is null" << std::endl;
        return;
    }

    if (memory_sim_info.lock())
    {
        if (tmp->num_updates <= 0)
        {
            memory_sim_info.unlock();
            OSG_FATAL << "ERROR: shared memory isn't updated with sim info." << std::endl;
            std::cout << "ERROR: shared memory isn't updated with sim info." << std::endl;
            OSG_FATAL << "Try to wait for 10 seconds." << std::endl;
            std::cout << "Try to wait for update shared memory..." << std::endl;

            time = 0;

            while (time <= timeout)
            {
                if (memory_sim_info.lock())
                {
                    if (tmp->num_updates <= 0)
                    {
                        //OSG_FATAL << "ERROR: shared memory isn't updated with sim info (after " << i + 1 << "seconds)." << std::endl;
                        //std::cout << "ERROR: shared memory isn't updated with sim info (after " << i + 1 << "seconds)." << std::endl;
                        memory_sim_info.unlock();
                        continue;
                    }
                    else
                    {
                        break;
                    }
                }

                QThread::sleep(delta);
                time += delta;
            }
        }

        if (tmp->num_updates <= 0)
        {
            OSG_FATAL << "ERROR: shared memory isn't updated with sim info." << std::endl;
            std::cout << "ERROR: shared memory isn't updated with sim info." << std::endl;
            memory_sim_info.unlock();
            return;
        }

        memcpy(&info_data, tmp, sizeof (simulator_info_t));
        memory_sim_info.unlock();
        OSG_FATAL << "Got simulator info from shared memory" << std::endl;
        std::cout << "Got simulator info from shared memory" << std::endl;

        QString route_dir_tmp = QString::fromStdWString(info_data.route_info.route_dir_name);
        route_dir_tmp.resize(info_data.route_info.route_dir_name_length);
        settings.route_dir_name = route_dir_tmp.toStdString();
        OSG_FATAL << "Route directory name from shared memory: " << route_dir_tmp.toStdString() << std::endl;
        std::cout << "Route directory name from shared memory: " << route_dir_tmp.toStdString() << std::endl;
    }
    else
    {
        OSG_FATAL << "ERROR: Can't lock shared memory" << std::endl;
        std::cout << "ERROR: Can't lock shared memory" << std::endl;
    }
}
*/
//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool RouteViewer::loadRoute()
{
    if (settings.route_dir_name.empty())
    {
        OSG_FATAL << "ERROR: Route directory name is empty" << std::endl;
        std::cout << "ERROR: Route directory name is empty" << std::endl;
        return false;
    }

    FileSystem &fs = FileSystem::getInstance();
    std::string route_dir_path = fs.combinePath(fs.getRouteRootDir(), settings.route_dir_name);
    settings.route_dir_full_path = route_dir_path;
    std::string routeType = osgDB::findDataFile(route_dir_path + fs.separator() + "route-type");

    if (routeType.empty())
    {
        OSG_FATAL << "ERROR: File route-type is not found in route directory" << std::endl;
        std::cout << "ERROR: File route-type is not found in route directory" << std::endl;
        return false;
    }

    std::ifstream stream(routeType);

    if (!stream.is_open())
    {
        OSG_FATAL << "ERROR: Stream for route-type file is't open" << std::endl;
        std::cout << "ERROR: Stream for route-type file is't open" << std::endl;
        return false;
    }

    std::string routeExt = "";
    stream >> routeExt;

    if (routeExt.empty())
    {
        OSG_FATAL << "ERROR: Unknown route type" << std::endl;
        std::cout << "ERROR: Unknown route type" << std::endl;
        return false;
    }

    std::string routeLoaderPlugin = routeExt + "-route-loader";

    osg::ref_ptr<RouteLoader> loader = loadRouteLoader(fs.getPluginsDir(), routeLoaderPlugin);

    if (!loader.valid())
    {
        OSG_FATAL << "ERROR: Not found route loader for this route" << std::endl;
        std::cout << "ERROR: Not found route loader for this route" << std::endl;
        return false;
    }

    std::cout << "Try loading route from " + route_dir_path << std::endl;
    loader->load(route_dir_path, settings.view_distance);
    std::cout << "Loaded route from " + route_dir_path << std::endl;

    root->addChild(loader->getRoot());
    return true;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool RouteViewer::loadVehicles(simulator_vehicles_info_t vehicles_info)
{
    if (vehicles_info.vehicles.empty())
    {
        OSG_FATAL << "ERROR: Server has not any vehicles" << std::endl;
        std::cout << "ERROR: Server has not any vehicles" << std::endl;
        return false;
    }

    train_ext_handler->load(vehicles_info);
    viewer.addEventHandler(train_ext_handler);
    QObject::connect(train_ext_handler, &TrainExteriorHandler::sendControlledVehicle,
                     this, &RouteViewer::slotUpdateControlledVehicle);

    std::vector<AnimationManager *> anims_manager = train_ext_handler->getAnimManagers();

    for (auto am : anims_manager)
    {
        viewer.addEventHandler(am);
    }

    root->addChild(train_ext_handler->getExterior());
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
    OSG_FATAL << "Starting init TCP-client" << std::endl;

    connect(tcp_client, &TcpClient::connected,
            this, &RouteViewer::slotConnectedToSimulator);

    connect(tcp_client, &TcpClient::setRouteInfo,
            this, &RouteViewer::slotGetRouteInfoData);

    connect(tcp_client, &TcpClient::setSignalsData,
            this, &RouteViewer::slotGetSignalsData);

    connect(tcp_client, &TcpClient::setVehiclesInfo,
            this, &RouteViewer::slotGetVehicleInfoData);

    connect(tcp_client, &TcpClient::sendLogMessage,
            this, &RouteViewer::slotRecvLogMessage);
/*
    connect(tcp_client, &TcpClient::setTrajBusyState,
            traffic_lights_handler,
            &TrafficLightsHandler::slotUpdateBusyData);
*/
    tcp_client->init(settings.tcp_config);

    OSG_FATAL << "TCP-lient is initilized...OK" << std::endl;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void RouteViewer::slotRecvLogMessage(QString msg)
{
    OSG_FATAL << msg.toStdString() << std::endl;
    std::cout << msg.toStdString() << std::endl;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void RouteViewer::slotConnectedToSimulator()
{
    OSG_FATAL << "Connected to server...OK" << std::endl;
    std::cout << "Connected to server...OK" << std::endl;

    OSG_FATAL << "Send request for route info" << std::endl;
    std::cout << "Send request for route info" << std::endl;
    tcp_client->sendRequest(STYPE_REQUEST_ROUTE_INFO);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void RouteViewer::slotGetRouteInfoData(QByteArray &data)
{
    if (is_route)
    {
        OSG_WARN << "WARNING: Get route info again" << std::endl;
        return;
    }
    is_route = true;

    simulator_route_info_t route_info;
    route_info.deserialize(data);
    settings.route_dir_name = route_info.route_dir_name.toStdString();
    OSG_FATAL << "Get route directory name: " << settings.route_dir_name << std::endl;
    std::cout << "Get route directory name: " << settings.route_dir_name << std::endl;

    // Load selected route
    if (!loadRoute())
    {
        OSG_FATAL << "Route from " << settings.route_dir_name << " is't loaded" << std::endl;
        std::cout << "Route from " << settings.route_dir_name << " is't loaded" << std::endl;
        exit(0);
    }

    OSG_FATAL << "Send request for signals data" << std::endl;
    std::cout << "Send request for signals data" << std::endl;
    tcp_client->sendRequest(STYPE_REQUEST_SIGNALS_DATA);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void RouteViewer::slotGetSignalsData(QByteArray &sig_data)
{
    if (is_signals)
    {
        OSG_WARN << "WARNING: Get signals data again" << std::endl;
        return;
    }
    is_signals = true;

    traffic_lights_handler->deserialize(sig_data);

    // Грузим модельки сигналов
    traffic_lights_handler->create_pagedLODs(settings);
    root->addChild(traffic_lights_handler->getSignalsGroup());

    traffic_lights_handler->load_signal_models(settings);

    for (auto am : traffic_lights_handler->animation_mangers)
    {
        viewer.addEventHandler(am);
    }

    // Добляем обработчик событий сигналов
    viewer.addEventHandler(traffic_lights_handler.get());

    connect(tcp_client, &TcpClient::updateSignal,
            traffic_lights_handler, &TrafficLightsHandler::slotUpdateSignal, Qt::DirectConnection);

    OSG_FATAL << "Send request for vehicles info" << std::endl;
    std::cout << "Send request for vehicles info" << std::endl;
    tcp_client->sendRequest(STYPE_REQUEST_VEHICLES_INFO);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void RouteViewer::slotGetVehicleInfoData(QByteArray &data)
{
    if (is_vehicles)
    {
        OSG_WARN << "WARNING: Get vehicles info again" << std::endl;
        return;
    }
    is_vehicles = true;

    simulator_vehicles_info_t vehicles_info;
    vehicles_info.deserialize(data);
    OSG_FATAL << "Get info about " << vehicles_info.vehicles.size() << " vehicles" << std::endl;
    std::cout << "Get info about " << vehicles_info.vehicles.size() << " vehicles" << std::endl;

    if (loadVehicles(vehicles_info))
    {
        connect(tcp_client, &TcpClient::setVehiclesPositions,
                train_ext_handler, &TrainExteriorHandler::slotGetVehiclesPosData, Qt::DirectConnection);

        connect(tcp_client, &TcpClient::setVehiclesData,
                train_ext_handler, &TrainExteriorHandler::slotGetVehiclesStateData, Qt::DirectConnection);

        connect(tcp_client, &TcpClient::setVehicleControlled,
                train_ext_handler, &TrainExteriorHandler::slotGetVehicleControlled, Qt::DirectConnection);

        vehicle_control_by_keyboard.controlled_vehicle = train_ext_handler->getControlledVehicle();
        vehicle_control_by_keyboard.current_vehicle = train_ext_handler->getCurrentVehicle();
        vehicle_control_by_keyboard.pressed_keys = keyboard->getPressedKeys();
        OSG_FATAL << "Send keyboard control to vehicle " << vehicle_control_by_keyboard.controlled_vehicle << std::endl;
        std::cout << "Send keyboard control to vehicle " << vehicle_control_by_keyboard.controlled_vehicle << std::endl;
        tcp_client->sendVehicleControl(vehicle_control_by_keyboard.serialize());

        OSG_FATAL << "Send request for continuous vehicles update" << std::endl;
        std::cout << "Send request for continuous vehicles update" << std::endl;
        tcp_client->sendRequest(STYPE_REQUEST_VEHICLES_POS_UPDATE,
                                static_cast<double>(settings.vehicles_pos_update_interval) / 1000.0);
        tcp_client->sendRequest(STYPE_REQUEST_VEHICLES_STATE_UPDATE,
                                static_cast<double>(settings.vehicles_state_update_interval) / 1000.0);
        tcp_client->sendRequest(STYPE_REQUEST_VEHICLE_CONTROLLED_UPDATE,
                                static_cast<double>(settings.vehicle_controled_update_interval) / 1000.0);
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void RouteViewer::slotUpdateKeyboard()
{
    vehicle_control_by_keyboard.pressed_keys = keyboard->getPressedKeys();
    tcp_client->sendVehicleControl(vehicle_control_by_keyboard.serialize());
/*
    QString msg = "Send keyboard: controlled ";
    msg += QString::number(vehicle_control_by_keyboard.controlled_vehicle);
    msg += " | current ";
    msg += QString::number(vehicle_control_by_keyboard.current_vehicle);
    msg += " | keys: ";
    msg += QString::number(vehicle_control_by_keyboard.pressed_keys.size());
    for (auto key_id : vehicle_control_by_keyboard.pressed_keys)
    {
        msg += " | ";
        msg += QString::number(key_id);
    }
    OSG_FATAL << msg.toStdString() << std::endl;
    std::cout << msg.toStdString() << std::endl;
*/
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void RouteViewer::slotUpdateControlledVehicle()
{
    vehicle_control_by_keyboard.controlled_vehicle = train_ext_handler->getControlledVehicle();
    vehicle_control_by_keyboard.current_vehicle = train_ext_handler->getCurrentVehicle();
    tcp_client->sendVehicleControl(vehicle_control_by_keyboard.serialize());
/*
    QString msg = "Send keyboard: controlled ";
    msg += QString::number(vehicle_control_by_keyboard.controlled_vehicle);
    msg += " | current ";
    msg += QString::number(vehicle_control_by_keyboard.current_vehicle);
    msg += " | keys: ";
    msg += QString::number(vehicle_control_by_keyboard.pressed_keys.size());
    for (auto key_id : vehicle_control_by_keyboard.pressed_keys)
    {
        msg += " | ";
        msg += QString::number(key_id);
    }
    OSG_FATAL << msg.toStdString() << std::endl;
    std::cout << msg.toStdString() << std::endl;
*/
}
