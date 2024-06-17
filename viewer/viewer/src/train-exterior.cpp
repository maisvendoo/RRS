//------------------------------------------------------------------------------
//
//      Loading and processing train exterior
//      (c) maisvendoo, 24/12/2018
//
//------------------------------------------------------------------------------
/*!
 * \file
 * \brief Loading and processing train exterior
 * \copyright maisvendoo
 * \author maisvendoo
 * \date 24/12/2018
 */

#include    "train-exterior.h"

#include    "config-reader.h"
#include    "get-value.h"
#include    "filesystem.h"
#include    "math-funcs.h"

#include    "vehicle-loader.h"

#include    <osgViewer/Viewer>

#include    <sstream>

#include    "anim-transform-visitor.h"
#include    <QDir>
#include    <QDirIterator>

#include    "model-animation.h"

#include    "display-loader.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
TrainExteriorHandler::TrainExteriorHandler(settings_t settings,
                                           MotionPath *routePath,
                                           const simulator_info_t &info_data)
    : QObject(Q_NULLPTR)
    , osgGA::GUIEventHandler ()
    , settings(settings)
    , cur_vehicle(0)
    , controlled_vehicle(-1)
    , long_shift(0.0f)
    , height_shift(0.0f)
    , routePath(routePath)
    , trainExterior(new osg::Group)
    , ref_time(0.0)
    , is_displays_locked(false)
    , new_data(-1)
    , old_data(-1)
    , memory_sim_update(nullptr)
    , memory_controlled(nullptr)
{
    load(info_data);

    memory_sim_update.setKey(SHARED_MEMORY_SIM_UPDATE);
    if (memory_sim_update.attach(QSharedMemory::ReadOnly))
    {
        OSG_FATAL << "Connected to shared memory with simulator update data" << std::endl;
    }
    else
    {
        OSG_FATAL << "Can't connect to shared memory with simulator update data" << std::endl;
    }

    memory_controlled.setKey(SHARED_MEMORY_CONTROLLED);
    if (memory_controlled.attach())
    {
        OSG_FATAL << "Connected to shared memory for info about controlled vehicle" << std::endl;
        controlled_t tmp;
        tmp.current_vehicle = cur_vehicle;
        tmp.controlled_vehicle = controlled_vehicle;
        sendControlledVehicle(tmp);
    }
    else
    {
        OSG_FATAL << "Can't connect to shared memory for info about controlled vehicle" << std::endl;
    }
/*
    shared_memory.setKey("sim");

    if (!shared_memory.attach(QSharedMemory::ReadOnly))
    {
        OSG_FATAL << "Can't connect to shared memory" << std::endl;
    }
*/
    //startTimer(settings.request_interval);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool TrainExteriorHandler::handle(const osgGA::GUIEventAdapter &ea,
                                  osgGA::GUIActionAdapter &aa)
{
    switch (ea.getEventType())
    {
    case osgGA::GUIEventAdapter::FRAME:
        {
            osgViewer::Viewer *viewer = dynamic_cast<osgViewer::Viewer *>(&aa);

            if (!viewer)
                break;

            ref_time += viewer->getFrameStamp()->getReferenceTime();

            processSharedData(ref_time);

            moveTrain(ref_time, update_data);

            moveCamera(viewer);

            break;
        }

    case osgGA::GUIEventAdapter::KEYDOWN:
        {
            keyboardHandler(ea.getKey());


            break;
        }

    default:

        break;
    }

    return false;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
osg::Group *TrainExteriorHandler::getExterior()
{
    if (trainExterior.valid())
        return trainExterior.get();

    return nullptr;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
std::vector<AnimationManager *> TrainExteriorHandler::getAnimManagers()
{
    return anim_managers;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TrainExteriorHandler::keyboardHandler(int key)
{
    controlled_t tmp;

    switch (key)
    {
    case osgGA::GUIEventAdapter::KEY_Page_Down:

        cur_vehicle++;

        if (cur_vehicle > static_cast<int>(vehicles_ext.size() - 1))
            cur_vehicle = 0;

        long_shift = 0.0f;

        tmp.current_vehicle = cur_vehicle;
        tmp.controlled_vehicle = controlled_vehicle;
        sendControlledVehicle(tmp);
        break;

    case osgGA::GUIEventAdapter::KEY_Page_Up:

        cur_vehicle--;

        if (cur_vehicle < 0)
            cur_vehicle = static_cast<int>(vehicles_ext.size() - 1);

        long_shift = 0.0f;

        tmp.current_vehicle = cur_vehicle;
        tmp.controlled_vehicle = controlled_vehicle;
        sendControlledVehicle(tmp);
        break;

    case osgGA::GUIEventAdapter::KEY_Home:

        long_shift += 0.5f;

        break;

    case osgGA::GUIEventAdapter::KEY_End:

        long_shift -= 0.5f;

        break;

    case osgGA::GUIEventAdapter::KEY_F2:

        cur_vehicle = 0;
        long_shift = 0.0f;
        height_shift = 0.0f;
        is_displays_locked = false;

        break;

    case osgGA::GUIEventAdapter::KEY_F3:

    case osgGA::GUIEventAdapter::KEY_F4:

    case osgGA::GUIEventAdapter::KEY_F5:

    case osgGA::GUIEventAdapter::KEY_F6:

        is_displays_locked = true;

        break;

    case osgGA::GUIEventAdapter::KEY_Delete:

        height_shift -= 0.05f;
        break;

    case osgGA::GUIEventAdapter::KEY_Insert:

        height_shift += 0.05f;
        break;

    case osgGA::GUIEventAdapter::KEY_KP_Enter:
    case osgGA::GUIEventAdapter::KEY_Return:

        controlled_vehicle = cur_vehicle;

        tmp.current_vehicle = cur_vehicle;
        tmp.controlled_vehicle = controlled_vehicle;
        sendControlledVehicle(tmp);
        break;
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TrainExteriorHandler::load(const simulator_info_t &info_data)
{
    int count = info_data.num_vehicles;

    for (int i = 0; i < count; ++i)
    {
        OSG_FATAL << "Vehicle " << i + 1 << " / " << count << " load" << std::endl;
        QString cfg_dir_tmp = QString::fromStdWString(info_data.vehicles_info[i].vehicle_config_dir);
        cfg_dir_tmp.resize(info_data.vehicles_info[i].vehicle_config_dir_length);
        std::string cfg_dir = cfg_dir_tmp.toStdString();

        QString cfg_file_tmp = QString::fromStdWString(info_data.vehicles_info[i].vehicle_config_file);
        cfg_file_tmp.resize(info_data.vehicles_info[i].vehicle_config_file_length);
        std::string cfg_file = cfg_file_tmp.toStdString();

        osg::ref_ptr<osg::Group> vehicle_model = loadVehicle(cfg_dir, cfg_file);

        if (!vehicle_model.valid())
        {
            OSG_FATAL << "Vehicle model from" << cfg_dir << "/" << cfg_file << " is't loaded" << std::endl;
            vehicle_exterior_t vehicle_ext = vehicle_exterior_t();
            vehicles_ext.push_back(vehicle_ext);
            OSG_FATAL << "Vehicle " << i + 1 << " / " << count << " added with empty model" << std::endl;
            continue;
        }
        OSG_FATAL << "Loaded vehicle model from" << cfg_dir << "/" << cfg_file << std::endl;

        // Load cabine model
        osg::ref_ptr<osg::Node> cabine;
        loadCabine(vehicle_model.get(), cfg_dir, cfg_file, cabine);

        osg::Vec3 driver_pos = getDirverPosition(cfg_dir, cfg_file);

        vehicle_exterior_t vehicle_ext = vehicle_exterior_t();
        vehicle_ext.transform->addChild(vehicle_model.get());
        vehicle_ext.cabine = cabine;
        vehicle_ext.driver_pos = driver_pos;

        loadModelAnimations(cfg_dir, cfg_file, vehicle_model.get(), *vehicle_ext.anims);
        loadAnimations(cfg_dir, cfg_file, vehicle_model.get(), *vehicle_ext.anims);
        loadAnimations(cfg_dir, cfg_file, cabine.get(), *vehicle_ext.anims);

        anim_managers.push_back(new AnimationManager(vehicle_ext.anims));

        loadDisplays(cfg_dir, cabine.get(), *vehicle_ext.displays);

        vehicles_ext.push_back(vehicle_ext);
        trainExterior->addChild(vehicle_ext.transform.get());
        OSG_FATAL << "Vehicle " << i + 1 << " / " << count << " loaded" << std::endl;
    }
    this->startTimer(100);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TrainExteriorHandler::moveTrain(double ref_time, const std::array<simulator_update_t, 2> sim_data)
{
    if ((old_data == -1) || (new_data == -1))
        return;

    // Time to relative units conversion
    float Delta_t = static_cast<float>(settings.request_interval) / 1000.0f;

    // Interframe coordinate
    float t = static_cast<float>(ref_time) / Delta_t;
    float k = (1.0f - t);

    for (size_t i = 0; i < vehicles_ext.size(); i++)
    {
        // Vehicle cartesian position and attitude calculation
        vehicles_ext[i].position = osg::Vec3(
            k * sim_data[old_data].vehicles[i].position_x + t * sim_data[new_data].vehicles[i].position_x,
            k * sim_data[old_data].vehicles[i].position_y + t * sim_data[new_data].vehicles[i].position_y,
            k * sim_data[old_data].vehicles[i].position_z + t * sim_data[new_data].vehicles[i].position_z);

        vehicles_ext[i].orth = osg::Vec3(
            k * sim_data[old_data].vehicles[i].orth_x + t * sim_data[new_data].vehicles[i].orth_x,
            k * sim_data[old_data].vehicles[i].orth_y + t * sim_data[new_data].vehicles[i].orth_y,
            k * sim_data[old_data].vehicles[i].orth_z + t * sim_data[new_data].vehicles[i].orth_z);

        vehicles_ext[i].up = osg::Vec3(
            k * sim_data[old_data].vehicles[i].up_x + t * sim_data[new_data].vehicles[i].up_x,
            k * sim_data[old_data].vehicles[i].up_y + t * sim_data[new_data].vehicles[i].up_y,
            k * sim_data[old_data].vehicles[i].up_z + t * sim_data[new_data].vehicles[i].up_z);

        vehicles_ext[i].attitude = osg::Vec3(
            asinf(vehicles_ext[i].orth.z()),
            0.0f,
            (vehicles_ext[i].orth.x() > 0.0) ? acos(vehicles_ext[i].orth.y()) : - acos(vehicles_ext[i].orth.y()) );

        vehicles_ext[i].orientation = update_data[new_data].vehicles[i].orientation;
/*
        // Store current railway coordinate and wheels angle
        recalcAttitude(i);
*/
        // Apply vehicle body matrix transform
        osg::Matrix  matrix;
        matrix *= osg::Matrixf::rotate(settings.direction * vehicles_ext[i].orientation * vehicles_ext[i].attitude.x(), osg::Vec3(1.0f, 0.0f, 0.0f));
        matrix *= osg::Matrixf::rotate(-vehicles_ext[i].attitude.z(), osg::Vec3(0.0f, 0.0f, 1.0f));
        matrix *= osg::Matrixf::translate(vehicles_ext[i].position);

        vehicles_ext[i].transform->setMatrix(matrix);

        for (auto it = vehicles_ext[i].anims->begin(); it != vehicles_ext[i].anims->end(); ++it)
        {
            ProcAnimation *animation = it.value();
            animation->setPosition(update_data[new_data].vehicles[i].analogSignal[animation->getSignalID()]);
        }
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TrainExteriorHandler::processSharedData(double &ref_time)
{
    if (ref_time < static_cast<double>(settings.request_interval) / 1000.0)
        return;

    ref_time = 0;

    if (memory_sim_update.lock())
    {
        simulator_update_t *sd = static_cast<simulator_update_t *>(memory_sim_update.data());

        if (sd == nullptr)
        {
            memory_sim_update.unlock();
            return;
        }

        if (old_data == -1)
        {
            if (new_data == -1)
            {
                memcpy(update_data.data(), sd, sizeof (simulator_update_t));
                new_data = 0;
            }
            else
            {
                memcpy(update_data.data() + 1, sd, sizeof (simulator_update_t));
                old_data = 0;
                new_data = 1;
            }
        }
        else
        {
            if (new_data == 1)
            {
                memcpy(update_data.data(), sd, sizeof (simulator_update_t));
                old_data = 1;
                new_data = 0;
            }
            else
            {
                memcpy(update_data.data() + 1, sd, sizeof (simulator_update_t));
                old_data = 0;
                new_data = 1;
            }
        }
        memory_sim_update.unlock();

        int seconds = static_cast<int>(std::floor(update_data[new_data].time));
        int hours = seconds / 3600;
        int minutes = seconds / 60 % 60;
        seconds = seconds % 60;
        QString hud_text = QString("Время от начала симуляции: %1 сек (%2 ч %3 м %4 c)\n")
                               .arg(update_data[new_data].time, 8, 'f', 1)
                               .arg(hours, 2)
                               .arg(minutes, 2)
                               .arg(seconds, 2);

        int curr = update_data[new_data].current_vehicle;
        if (curr >= 0)
        {
            hud_text += QString("Данная ПЕ: %1 | pos{%2,%3,%4} | dir{%5,%6,%7}\n")
                            .arg(curr, 3)
                            .arg(update_data[new_data].vehicles[curr].position_x, 8, 'f', 1)
                            .arg(update_data[new_data].vehicles[curr].position_y, 8, 'f', 1)
                            .arg(update_data[new_data].vehicles[curr].position_z, 8, 'f', 1)
                            .arg(update_data[new_data].vehicles[curr].orth_x, 6, 'f', 3)
                            .arg(update_data[new_data].vehicles[curr].orth_y, 6, 'f', 3)
                            .arg(update_data[new_data].vehicles[curr].orth_z, 6, 'f', 3);

            hud_text += QString::fromStdWString(update_data[new_data].currentDebugMsg) + QString("\n");
        }
        else
        {
            hud_text += QString("\n\n");
        }

        int control = update_data[new_data].controlled_vehicle;
        if (control >= 0)
        {
            hud_text += QString("Управляемая ПЕ: %1 | pos{%2,%3,%4} | dir{%5,%6,%7}\n")
                            .arg(control, 3)
                            .arg(update_data[new_data].vehicles[control].position_x, 8, 'f', 1)
                            .arg(update_data[new_data].vehicles[control].position_y, 8, 'f', 1)
                            .arg(update_data[new_data].vehicles[control].position_z, 8, 'f', 1)
                            .arg(update_data[new_data].vehicles[control].orth_x, 6, 'f', 3)
                            .arg(update_data[new_data].vehicles[control].orth_y, 6, 'f', 3)
                            .arg(update_data[new_data].vehicles[control].orth_z, 6, 'f', 3);

            hud_text += QString::fromStdWString(update_data[new_data].controlledDebugMsg);
        }
        else
        {
            hud_text += QString("Управляемая ПЕ: не выбрана\nНажмите Enter, чтобы управлять данной ПЕ");
        }

        std::wstring text = hud_text.toStdWString();
        emit setStatusBar(text);
    }
/*
    if (shared_memory.lock())
    {
        server_data_t *sd = static_cast<server_data_t *>(shared_memory.data());

        if (sd == nullptr)
        {
            shared_memory.unlock();
            return;
        }

        server_data_t server_data;

        memcpy(&server_data, sd, sizeof (server_data_t));

        nd.sd.push_back(server_data);

        if (nd.sd.size() > 2)
        {
            nd.sd.pop_front();
            nd.delta_time = nd.sd.back().time - nd.sd.front().time;
        }

        QString msg = QString("ПЕ #%1: ")
                .arg(cur_vehicle);

        emit setStatusBar(msg + QString::fromStdWString(server_data.te[static_cast<size_t>(cur_vehicle)].DebugMsg));

        shared_memory.unlock();
    }
*/
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TrainExteriorHandler::sendControlledVehicle(const controlled_t &data)
{
    if (memory_controlled.lock())
    {
        controlled_t *c = static_cast<controlled_t *>(memory_controlled.data());

        if (c == nullptr)
        {
            memory_controlled.unlock();
            return;
        }

        memcpy(memory_controlled.data(), &data, sizeof(controlled_t));

        memory_controlled.unlock();
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TrainExteriorHandler::moveCamera(osgViewer::Viewer *viewer)
{
    Q_UNUSED(viewer)

    camera_position_t cp;
    cp.position = vehicles_ext[static_cast<size_t>(cur_vehicle)].position;
    cp.attitude = vehicles_ext[static_cast<size_t>(cur_vehicle)].attitude;
    cp.driver_pos = vehicles_ext[static_cast<size_t>(cur_vehicle)].driver_pos;
    cp.is_orient_bwd = (vehicles_ext[static_cast<size_t>(cur_vehicle)].orientation < 0);

    cp.attitude.x() = - osg::PIf / 2.0f - cp.attitude.x() * settings.direction;
/*
    float viewer_coord = vehicles_ext[static_cast<size_t>(cur_vehicle)].coord +
            settings.direction * settings.stat_cam_shift;

    cp.viewer_pos = routePath->getPosition(viewer_coord, cp.view_basis);
*/
    cp.viewer_pos = vehicles_ext[static_cast<size_t>(cur_vehicle)].position
        + vehicles_ext[static_cast<size_t>(cur_vehicle)].orth * settings.direction * settings.stat_cam_shift;

    cp.view_basis.front = vehicles_ext[static_cast<size_t>(cur_vehicle)].orth;
    cp.view_basis.right = cp.view_basis.front ^ osg::Vec3(osg::Z_AXIS);

    emit sendCameraPosition(cp);
}
/*
//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TrainExteriorHandler::recalcAttitude(size_t i)
{
    // Attitude.z from -PI to +PI
    while (abs(vehicles_ext[i].attitude.z()) > osg::PIf)
        vehicles_ext[i].attitude.z() -= 2.0f * osg::PIf * sign(vehicles_ext[i].attitude.z());

    // Don't recalculate from head vehicle
    if (i == 0)
        return;

    // Get prevuos vehicle
    vehicle_exterior_t prev = vehicles_ext[i-1];
    // Get current vehicles
    vehicle_exterior_t curr = vehicles_ext[i];

    // Calculate vehicle tail orth
    osg::Vec3 prev_att = prev.attitude;
    float pitch = prev_att.x();
    float yaw = prev_att.z();
//    yaw -= osg::PIf * hs_n(prev.orientation);
    osg::Vec3 tail_orth = osg::Vec3(-cosf(pitch) * sinf(yaw), -cosf(pitch) * cosf(yaw), -sinf(pitch));

    // Calculate bacward coupling point of previos vehicle
//    osg::Vec3 tail_dir = tail_orth * (prev.length / 2.0f);

    // Calculate forward coupling of current vehicle orth
//    osg::Vec3 a = prev.position + tail_dir;
    osg::Vec3 a = prev.position;
    osg::Vec3 forward = a - curr.position;
    osg::Vec3 f_orth = forward * (1 / forward.length());

    // Calculate new attitude of current vehicle
    float y_new = arg(f_orth.y(), f_orth.x());
    y_new -= osg::PIf * osg::sign(y_new) * hs_n(curr.orientation);
    float y_old = curr.attitude.z();

    // Change y_old for correct work near -PI and +PI
    float dy = y_new - y_old;
    if (fabs(dy) > osg::PIf)
    {
        y_old += 2.0 * osg::PIf * osg::sign(dy);
    }

    // "Smoothing" of vehicle oscillations
    vehicles_ext[i].attitude.z() = (y_new + y_old) / 2.0;
}
*/
//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TrainExteriorHandler::loadAnimations(const std::string &configDir,
                                          const std::string &configName,
                                          osg::Node *cabine,
                                          animations_t &animations)
{
    if (cabine == nullptr)
        return;

    // Default name of animations configs directory is name of vehicle's config
    std::string anim_config_dir = configName;

    FileSystem &fs = FileSystem::getInstance();
    std::string relative_cfg_path = configDir + fs.separator() + configName + ".xml";
    std::string cfg_path = fs.combinePath(fs.getVehiclesDir(), relative_cfg_path);

    // Load config file
    ConfigReader cfg(cfg_path);

    if (cfg.isOpenned())
    {
        std::string secName = "Vehicle";
        cfg.getValue(secName, "AnimationsConfigDir", anim_config_dir);
        OSG_FATAL << "Vehicle animations are loaded from " << anim_config_dir << std::endl;
    }

    AnimTransformVisitor atv(&animations, anim_config_dir);
    atv.setTraversalMode(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN);

    cabine->accept(atv);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TrainExteriorHandler::loadModelAnimations(const std::string &configDir,
                                               const std::string &configName,
                                               osg::Node *model,
                                               animations_t &animations)
{
    // Default name of animations configs directory is name of vehicle's config
    std::string anim_config_dir = configName;

    FileSystem &fs = FileSystem::getInstance();
    std::string relative_cfg_path = configDir + fs.separator() + configName + ".xml";
    std::string cfg_path = fs.combinePath(fs.getVehiclesDir(), relative_cfg_path);

    // Load config file
    ConfigReader cfg(cfg_path);

    if (cfg.isOpenned())
    {
        std::string secName = "Vehicle";
        cfg.getValue(secName, "AnimationsConfigDir", anim_config_dir);
        OSG_FATAL << "Vehicle animations are loaded from " << anim_config_dir << std::endl;
    }

    std::string animations_dir = fs.combinePath(fs.getDataDir(), "animations");
    animations_dir = fs.combinePath(animations_dir, anim_config_dir);

    QDir animDir(QString(animations_dir.c_str()));

    QDirIterator animFiles(animDir.path(),
                           QStringList() << "*.xml",
                           QDir::NoDotAndDotDot | QDir::Files);

    while (animFiles.hasNext())
    {
        QString fullPath = animFiles.next();
        QFileInfo fileInfo(fullPath);

        QString animation_name = fileInfo.baseName();

        ConfigReader cfg(fullPath.toStdString());

        if (!cfg.isOpenned())
        {
            continue;
        }

        osgDB::XmlNode *rootNode = cfg.getConfigNode();

        for (auto it = rootNode->children.begin(); it != rootNode->children.end(); ++it)
        {
            osgDB::XmlNode  *child = *it;

            if (child->name == "ModelAnimation")
            {
                ModelAnimation *animation = new ModelAnimation(model, animation_name.toStdString());
                animation->load(cfg);
                animations.insert(animation->getSignalID(), animation);
            }
        }
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TrainExteriorHandler::loadDisplays(const std::string &configDir,
                                        osg::Node *model,
                                        displays_t &displays)
{
    // Определяем имя конфига для формирования пути к файлу displays.xml,
    // где хранится список дисплеев
    FileSystem &fs = FileSystem::getInstance();
    std::string vehicle_config_dir = fs.combinePath(fs.getVehiclesDir(), configDir); ;
    std::string cfg_path = vehicle_config_dir + fs.separator() + "displays.xml";

    ConfigReader displays_cfg(cfg_path);

    if (!displays_cfg.isOpenned())
    {
        OSG_FATAL << "File " << cfg_path << " is't found" << std::endl;
        return;
    }

    osgDB::XmlNode *config_node = displays_cfg.getConfigNode();

    for (auto it = config_node->children.begin(); it != config_node->children.end(); ++it)
    {
        osgDB::XmlNode *display_node = *it;

        if (display_node->name == "Display")
        {
            display_config_t display_config;

            osgDB::XmlNode *module_dir_node = displays_cfg.findSection(display_node, "ModuleDir");
            std::string module_dir = fs.combinePath(fs.getModulesDir(), module_dir_node->contents);

            osgDB::XmlNode *module_node = displays_cfg.findSection(display_node, "Module");
            std::string module_path = fs.combinePath(module_dir, module_node->contents);
            display_config.module_name = QString(module_path.c_str());

            osgDB::XmlNode *surface_name_node = displays_cfg.findSection(display_node, "SurfaceName");
            display_config.surface_name = QString(surface_name_node->contents.c_str());

            display_config.texcoord = new osg::Vec2Array;
            size_t i = 0;

            QString corner_name = QString("Corner%1").arg(i+1);
            osgDB::XmlNode *corner_node = displays_cfg.findSection(display_node, corner_name.toStdString());

            while (corner_node != nullptr)
            {
                std::stringstream ss(corner_node->contents);
                float x, y;
                ss >> x >> y;
                osg::Vec2 texel(x, y);
                display_config.texcoord->push_back(texel);

                i++;
                QString corner_name = QString("Corner%1").arg(i+1);
                corner_node = displays_cfg.findSection(display_node, corner_name.toStdString());
            }

            display_container_t *dc = new display_container_t();

            loadDisplayModule(display_config, dc, model);

            if (dc->display == nullptr)
                continue;

            dc->display->setConfigDir(QString(vehicle_config_dir.c_str()));
            dc->display->setRouteDir(QString(settings.route_dir.c_str()));
            dc->display->init();

            displays.push_back(dc);
        }
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TrainExteriorHandler::timerEvent(QTimerEvent *)
{
    if ((old_data == -1) || (new_data == -1))
        return;

    if (is_displays_locked)
        return;

    for (size_t i = 0; i < vehicles_ext.size(); ++i)
    {
        for (auto it = vehicles_ext[i].displays->begin(); it != vehicles_ext[i].displays->end(); ++it)
        {
            display_container_t *dc = *it;
            dc->display->setInputSignals(update_data[new_data].vehicles[i].analogSignal);
        }
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TrainExteriorHandler::lock_display(bool lock)
{
    is_displays_locked = lock;
}
