#include "RouteViewer.h"

#include "CLI11.hpp"
#include "DmdReaderWriter.h"
#include "Route.h"
#include "RouteLoader.h"
#include "simulator-info-struct.h"
#include "ConfigReader.h"
#include "SoundManager.h"
#include "TrainExteriorHandler.h"
#include "cmd-line.h"
#include "filesystem.h"
#include "network-data-types.h"
#include "settings.h"
#include "Logger.h"
#include "tcp-client.h"

#include <cmath>
#include <iostream>
#include <memory>
#include <QApplication>
#include <QObject>
#include <sstream>

#include <string>
#include <vsg/all.h>
#include <vsgXchange/all.h>

RouteViewer::RouteViewer(int argc, char* argv[], QObject* parent)
    : QObject(parent)
    , is_ready(false)
    , is_route(false)
    , tcp_client(new TcpClient(this))
{
    if (init(argc, argv))
    {
        LOG_INFO("Viewer is initialized succesfully");
        is_ready = true;
    }
    else
    {
        LOG_FATAL("Fail to initialize viewer");
    }
}

RouteViewer::~RouteViewer() = default;

bool RouteViewer::isReady() const
{
    return true;
}

int RouteViewer::run()
{
    while (viewer->advanceToNextFrame())
    {
        QApplication::processEvents();

        viewer->handleEvents();
        viewer->update();
        viewer->recordAndSubmit();
        viewer->present();

        LOG_INFO("%f %f %f", lookAt->eye.x, lookAt->eye.y, lookAt->eye.z);
    }

    return 0;
}

bool RouteViewer::init(int argc, char* argv[])
{
    FileSystem& fs = FileSystem::getInstance();

    Logger::instance().openFile(fs.getLogsDir() + fs.separator() + "viewer.log");

    // osgDB::DatabasePager *dp = viewer.getDatabasePager();
    // dp->setDoPreCompile(true);
    // dp->setTargetMaximumNumberOfPageLOD(1000);

    loadSettings(fs.getConfigDir() + fs.separator() + "settings.xml");
    LOG_INFO("Loaded settings from settings.xml");

    LogLevel level = LOG_LEVEL_INFO;

    if (settings.notify_level == "INFO")
    {
        level = LOG_LEVEL_INFO;
    }
    else if (settings.notify_level == "WARN")
    {
        level = LOG_LEVEL_WARN;
    }
    else if (settings.notify_level == "FATAL")
    {
        level = LOG_LEVEL_FATAL;
    }

    Logger::instance().level = level;

    LOG_INFO("Override settings from command line");
    overrideSettingsByCommandLine(argc, argv);

    sound_manager = std::make_unique<SoundManager>();
    LOG_INFO("Created SoundManager");

    train_ext_handler = std::make_unique<TrainExteriorHandler>(settings, sound_manager);

    root = vsg::Group::create();

    if (!initEngineSettings())
    {
        return false;
    }

    if (!initDisplay())
    {
        return false;
    }

    initTCPclient();

    /*

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

    */

    return true;
}

void RouteViewer::loadSettings(const std::string& cfg_path)
{
    try
    {
        ConfigReader cfg(cfg_path);

        cfg.setSection("Client");

        std::string host_addr;
        cfg.getValue("HostAddr", host_addr);
        settings.tcp_config.host_addr = host_addr.c_str();

        cfg.getValue("port", settings.tcp_config.port);

        LOG_INFO("Host for client from settings: %s:%u", host_addr.c_str(), settings.tcp_config.port);

        cfg.getValue("ReconnectInterval", settings.tcp_config.reconnect_interval);
        cfg.getValue("VehiclesPosUpdateInterval", settings.vehicles_pos_update_interval);
        cfg.getValue("VehiclesStateUpdateInterval", settings.vehicles_state_update_interval);
        cfg.getValue("VehicleControlledUpdateInterval", settings.vehicle_controled_update_interval);
        cfg.getValue("ClientDelay", settings.client_delay);

        cfg.setSection("Viewer");

        cfg.getValue("Width", settings.width);
        cfg.getValue("Height", settings.height);
        cfg.getValue("FullScreen", settings.fullscreen);
        cfg.getValue("VSync", settings.vsync);
        cfg.getValue("posX", settings.x);
        cfg.getValue("posY", settings.y);
        cfg.getValue("FovY", settings.fovy);
        cfg.getValue("zNear", settings.zNear);
        cfg.getValue("zFar", settings.zFar);
        cfg.getValue("ScreenNumber", settings.screen_number);
        cfg.getValue("WindowDecoration", settings.window_decoration);
        cfg.getValue("DoubleBuffer", settings.double_buffer);
        cfg.getValue("Samples", settings.samples);
        cfg.getValue("MotionBlur", settings.persistence);
        cfg.getValue("NotifyLevel", settings.notify_level);
        cfg.getValue("ViewDistance", settings.view_distance);

        cfg.getValue("CabineCamRotCoeff", settings.cabine_cam_rot_coeff);
        cfg.getValue("CabineCamFovYStep", settings.cabine_cam_fovy_step);
        cfg.getValue("CabineCamSpeed", settings.cabine_cam_speed);

        cfg.getValue("ExtCamInitDist", settings.ext_cam_init_dist);
        cfg.getValue("ExtCamInitHeight", settings.ext_cam_init_height);
        cfg.getValue("ExtCamInitShift", settings.ext_cam_init_shift);
        cfg.getValue("ExtCamRotCoeff", settings.ext_cam_rot_coeff);
        cfg.getValue("ExtCamSpeed", settings.ext_cam_speed);
        cfg.getValue("ExtCamSpeedCoeff", settings.ext_cam_speed_coeff);
        cfg.getValue("ExtCamMinDist", settings.ext_cam_min_dist);
        cfg.getValue("ExtCamInitAngleH", settings.ext_cam_init_angle_H);
        cfg.getValue("ExtCamInitAngleV", settings.ext_cam_init_angle_V);

        std::string free_cam_init_pos;
        cfg.getValue("FreeCamInitPos", free_cam_init_pos);
        if (!free_cam_init_pos.empty())
        {
            std::istringstream stream(free_cam_init_pos);
            stream >> settings.free_cam_init_pos.x
                >> settings.free_cam_init_pos.y
                >> settings.free_cam_init_pos.z;
        }

        cfg.getValue("FreeCamRotCoeff", settings.free_cam_rot_coeff);
        cfg.getValue("FreeCamSpeed", settings.free_cam_speed);
        cfg.getValue("FreeCamSpeedCoeff", settings.free_cam_speed_coeff);
        cfg.getValue("FreeCamFovY", settings.free_cam_fovy_step);

        cfg.getValue("StatCamDist", settings.stat_cam_dist);
        cfg.getValue("StatCamHeight", settings.stat_cam_height);
        cfg.getValue("StatCamShift", settings.stat_cam_shift);

        cfg.getValue("FrameDiv", settings.interval);
    }
    catch (...)
    {
    }
}

int RouteViewer::overrideSettingsByCommandLine(int argc, char* argv[])
{
    cmd_line_t cmd_line;

    CLI::App app("Viewer");
    argv = app.ensure_utf8(argv);

    app.add_option("--route", cmd_line.route_dir);
    app.add_option("--train", cmd_line.train_config);
    app.add_option("--host-addr", cmd_line.host_addr);
    app.add_option("--port", cmd_line.port);
    app.add_option("--width", cmd_line.width);
    app.add_option("--height", cmd_line.height);
    app.add_option("--direction", cmd_line.direction);
    app.add_flag("--fullscreen", cmd_line.fullscreen);
    app.add_flag("--localmode", cmd_line.localmode);
    app.add_option("--notify-level", cmd_line.notify_level);

    CLI11_PARSE(app, argc, argv);

    if (cmd_line.host_addr)
    {
        settings.tcp_config.host_addr = cmd_line.host_addr->c_str();
    }

    if (cmd_line.port)
    {
        settings.tcp_config.port = static_cast<quint16>(cmd_line.port.value());
    }

    if (cmd_line.width)
    {
        settings.width = cmd_line.width.value();
    }

    if (cmd_line.height)
    {
        settings.height = cmd_line.height.value();
    }

    settings.fullscreen = cmd_line.fullscreen;

    if (cmd_line.notify_level)
    {
        settings.notify_level = cmd_line.notify_level.value();
    }

    if (cmd_line.direction)
    {
        settings.direction = cmd_line.direction.value();
    }

    if (cmd_line.route_dir)
    {
        settings.route_dir_name = cmd_line.route_dir.value();
    }

    return 0;
}

bool RouteViewer::initEngineSettings()
{
    if (!root)
    {
        return false;
    }

    initEnvironmentLight(vsg::vec4(1.0f, 1.0f, 1.0f, 1.0f), 1.0f, -20.0f, 75.0f);

    return true;

    /*
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
    */
}

void RouteViewer::initEnvironmentLight(vsg::vec4 color, float power, float psi, float theta)
{
    auto sun = vsg::DirectionalLight::create();
    sun->color = vsg::vec3(color.r, color.g, color.b);
    sun->intensity = power;

    float dist = 1000.0f;
    float rad = vsg::PIf / 180.0f;
    float x = dist * std::cosf(theta * rad) * std::sinf(psi * rad);
    float y = dist * std::cosf(theta * rad) * std::cosf(psi * rad);
    float z = dist * std::sinf(theta * rad);

    vsg::vec3 pos(x, y, z);
    // sun->position.set(x, y, z);

    float length = vsg::length(pos);
    vsg::vec3 sunDir = pos;
    sunDir *= (-1.0f / length);
    sun->direction = sunDir;

    root->addChild(sun);

    /*
    osg::ref_ptr<osg::Light> sun = new osg::Light;
    sun->setLightNum(0);
    sun->setDiffuse(color *= power);
    sun->setAmbient(color *= power * 0.0f);
    sun->setSpecular(color *= power);


    float dist = 1000.0f;

    float rad = osg::PIf / 180.0f;
    float x = dist * cosf(theta * rad) * sinf(psi * rad);
    float y = dist * cosf(theta * rad) * cosf(psi * rad);
    float z = dist * sinf(theta * rad);

    osg::Vec3 pos = osg::Vec3(x, y, z);
    sun->setPosition(osg::Vec4(pos, 0.0f));

    osg::Vec3 sunDir = pos *= (- 1.0f / pos.length());
    sun->setDirection(sunDir);

    osg::ref_ptr<osg::LightSource> light0 = new osg::LightSource;
    light0->setLight(sun);

    root->getOrCreateStateSet()->setMode(GL_LIGHT0, osg::StateAttribute::ON);
    root->addChild(light0.get());
    */
}

bool RouteViewer::initDisplay()
{
    options = vsg::Options::create();
    options->fileCache = vsg::getEnv("VSG_FILE_CACHE");
    options->paths = vsg::getEnvPaths("VSG_FILE_PATH");
    options->paths.push_back("/home/ksv/work-ANI/Projects/ANI/RRS/");
    options->add(vsgXchange::all::create());
    options->add(DmdReaderWriter::create());
    options->sharedObjects = vsg::SharedObjects::create();

    viewer = vsg::Viewer::create();

    auto traits = vsg::WindowTraits::create();
    traits->x = settings.x;
    traits->y = settings.y;
    traits->width = settings.width;
    traits->height = settings.height;
    traits->windowTitle = settings.name;
    traits->decoration = settings.window_decoration;
    traits->samples = settings.samples;
    traits->debugLayer = true;
    traits->debugUtils = true;

    window = vsg::Window::create(traits);
    viewer->addWindow(window);

    initCamera();

    viewer->addEventHandler(vsg::CloseHandler::create(viewer));
    viewer->addEventHandler(vsg::Trackball::create(camera));

    auto commandGraph = vsg::createCommandGraphForView(window, camera, root);
    viewer->assignRecordAndSubmitTaskAndPresentation({commandGraph});
    viewer->compile();

    return true;
}

void RouteViewer::initCamera()
{
    vsg::ComputeBounds computeBounds;
    root->accept(computeBounds);
    vsg::dvec3 centre(0.0, 0.0, 0.0);
    double radius = 100.0;
    double nearFarRatio = 0.0005;

    double aspect_ratio = static_cast<double>(window->extent2D().width)
        / static_cast<double>(window->extent2D().height);

    auto perspective = vsg::Perspective::create(settings.fovy, aspect_ratio, nearFarRatio * radius, radius * 4.5);

    lookAt = vsg::LookAt::create(
        centre + vsg::dvec3(0.0, -radius * 3.5, 20.0),
        centre,
        vsg::dvec3(0.0, 0.0, 1.0)
    );

    camera = vsg::Camera::create(perspective, lookAt, vsg::ViewportState::create(window->extent2D()));
}

void RouteViewer::initTCPclient()
{
    LOG_INFO("Starting init TCP-client");

    connect(tcp_client, &TcpClient::connected, this, &RouteViewer::slotConnectedToSimulator);
    connect(tcp_client, &TcpClient::setRouteInfo, this, &RouteViewer::slotGetRouteInfoData);
    connect(tcp_client, &TcpClient::setSignalsData, this, &RouteViewer::slotGetSignalsData);
    connect(tcp_client, &TcpClient::setVehiclesInfo, this, &RouteViewer::slotGetVehicleInfoData);
    connect(tcp_client, &TcpClient::sendLogMessage, this, &RouteViewer::slotRecvLogMessage);

    tcp_client->init(settings.tcp_config);

    LOG_INFO("TCP-client is initialized...OK");
}

bool RouteViewer::loadRoute()
{
    if (settings.route_dir_name.empty())
    {
        LOG_ERROR("Route directory name is empty");
        return false;
    }

    FileSystem& fs = FileSystem::getInstance();
    std::string route_dir_path = fs.combinePath(fs.getRouteRootDir(), settings.route_dir_name);
    settings.route_dir_full_path = route_dir_path;

    Route route;

    RouteLoader loader(settings.route_dir_full_path);
    loader.read_description();
    loader.parse_objects_ref(route);
    loader.parse_route_map(route);

    for (auto& [label, transform] : route.transforms)
    {
        auto found_it = route.object_ref.find(label);
        if (found_it == route.object_ref.end())
        {
            continue;
        }

        vsg::dvec3 center(transform.t_x, transform.t_y, 10.0);
        double radius = 100.0;

        auto pagedLOD = vsg::PagedLOD::create();
        vsg::dsphere bound;
        bound.center = center;
        bound.radius = radius;
        pagedLOD->bound = bound;
        pagedLOD->filename = route_dir_path + found_it->second;
        pagedLOD->options = options;

        // auto node = vsg::Node::create();

        // auto external = vsg::External::create();
        // external->options = options;
        // external->add(route_dir_path + found_it->second, node);

        // auto pagedLOD = vsg::PagedLOD::create();
        // pagedLOD->children[0].minimumScreenHeightRatio = 0.0;
        // pagedLOD->children[0].node = node;

        auto matrix = vsg::MatrixTransform::create();
        transform.r_x = -vsg::radians(transform.r_x);
        transform.r_y = -vsg::radians(transform.r_y);
        transform.r_z = -vsg::radians(transform.r_z);

        auto rotate_x = vsg::rotate(transform.r_x, vsg::vec3(1.0f, 0.0f, 0.0f));
        auto rotate_y = vsg::rotate(transform.r_y, vsg::vec3(0.0f, 1.0f, 0.0f));
        auto rotate_z = vsg::rotate(transform.r_z, vsg::vec3(0.0f, 0.0f, 1.0f));
        auto translate = vsg::translate(transform.t_x, transform.t_y, transform.t_z);
        matrix->matrix = translate * rotate_z * rotate_y * rotate_x;

        matrix->addChild(pagedLOD);

        root->addChild(matrix);
    }

    viewer->update();
    viewer->compile();

    return true;
}

void RouteViewer::slotRecvLogMessage(QString msg)
{
    LOG_INFO("%s", msg.toStdString().c_str());
}

void RouteViewer::slotConnectedToSimulator()
{
    LOG_INFO("Connected to server...OK");
    LOG_INFO("Send request for route info");
    tcp_client->sendRequest(STYPE_REQUEST_ROUTE_INFO);
}

void RouteViewer::slotGetRouteInfoData(QByteArray &data)
{
    if (is_route)
    {
        LOG_WARN("Get route info again");
        return;
    }
    is_route = true;

    /*
    QString msg = QString("Загрузка маршрута...");
    imguiWidgetsHandler->setLoadingStatus(msg);
    */

    simulator_route_info_t route_info;
    route_info.deserialize(data);
    settings.route_dir_name = route_info.route_dir_name.toStdString();
    LOG_INFO("Get route directory name: %s", settings.route_dir_name.c_str());

    loadRoute();
}

void RouteViewer::slotGetSignalsData(QByteArray &sig_data)
{

}

void RouteViewer::slotGetVehicleInfoData(QByteArray &data)
{

}

void RouteViewer::slotUpdateKeyboard()
{

}

void RouteViewer::slotUpdateControlledVehicle()
{

}

