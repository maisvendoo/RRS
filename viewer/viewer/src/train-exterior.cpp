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
                                           SoundManager *sm,
                                           const simulator_info_t &info_data)
    : QObject(Q_NULLPTR)
    , osgGA::GUIEventHandler ()
    , settings(settings)
    , cur_vehicle(0)
    , controlled_vehicle(0)
    , long_shift(0.0f)
    , height_shift(0.0f)
    , trainExterior(new osg::Group)
    , prev_time(0.0)
    , ref_time(0.0)
    , is_displays_locked(false)
    , new_data(-1)
    , old_data(-1)
    , memory_sim_update(nullptr)
    , memory_controlled(nullptr)
    , sound_manager(sm)
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
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
TrainExteriorHandler::~TrainExteriorHandler()
{
    memory_sim_update.detach();
    memory_controlled.detach();
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

            double curr_time = viewer->getFrameStamp()->getReferenceTime();
            ref_time += curr_time - prev_time;
            prev_time = curr_time;

            processSharedData(ref_time);

            moveTrain(ref_time, update_data);

            moveCamera(viewer, curr_time - prev_time);

            break;
        }

    case osgGA::GUIEventAdapter::KEYDOWN:
        {
            int key = ea.getUnmodifiedKey();
            controlled_t tmp;
            switch (key)
            {
            case osgGA::GUIEventAdapter::KEY_Page_Down:

                cur_vehicle++;

                if (cur_vehicle > static_cast<int>(vehicles_ext.size() - 1))
                    cur_vehicle = 0;

                tmp.current_vehicle = cur_vehicle;
                tmp.controlled_vehicle = controlled_vehicle;
                sendControlledVehicle(tmp);

                break;

            case osgGA::GUIEventAdapter::KEY_Page_Up:

                cur_vehicle--;

                if (cur_vehicle < 0)
                    cur_vehicle = static_cast<int>(vehicles_ext.size() - 1);

                tmp.current_vehicle = cur_vehicle;
                tmp.controlled_vehicle = controlled_vehicle;
                sendControlledVehicle(tmp);

                break;

            case osgGA::GUIEventAdapter::KEY_KP_Enter:
            case osgGA::GUIEventAdapter::KEY_Return:

                controlled_vehicle = cur_vehicle;

                tmp.current_vehicle = cur_vehicle;
                tmp.controlled_vehicle = controlled_vehicle;
                sendControlledVehicle(tmp);
                break;

            case osgGA::GUIEventAdapter::KEY_Shift_L:
                is_Shift_L = true;
                return false;
            case osgGA::GUIEventAdapter::KEY_Shift_R:
                is_Shift_R = true;
                return false;
            case osgGA::GUIEventAdapter::KEY_Control_L:
                is_Ctrl_L = true;
                return false;
            case osgGA::GUIEventAdapter::KEY_Control_R:
                is_Ctrl_R = true;
                return false;
            case osgGA::GUIEventAdapter::KEY_Alt_L:
                is_Alt_L = true;
                return false;
            case osgGA::GUIEventAdapter::KEY_Alt_R:
                is_Alt_R = true;
                return false;

            case osgGA::GUIEventAdapter::KEY_F2:

                if (is_Shift_L || is_Shift_R || is_Ctrl_L || is_Ctrl_R || is_Alt_L || is_Alt_R)
                    return false;
                if (controlled_vehicle >= 0)
                    cur_vehicle = controlled_vehicle;
                is_displays_locked = false;

                tmp.current_vehicle = cur_vehicle;
                tmp.controlled_vehicle = controlled_vehicle;
                sendControlledVehicle(tmp);
                break;

            case osgGA::GUIEventAdapter::KEY_F3:
            case osgGA::GUIEventAdapter::KEY_F4:
            case osgGA::GUIEventAdapter::KEY_F5:
            case osgGA::GUIEventAdapter::KEY_F6:

                if (is_Shift_L || is_Shift_R || is_Ctrl_L || is_Ctrl_R || is_Alt_L || is_Alt_R)
                    return false;
                is_displays_locked = true;
                break;

            default: break;
            }
            break;
        }

    case osgGA::GUIEventAdapter::KEYUP:
    {
        int key = ea.getUnmodifiedKey();
        switch (key)
        {
        case osgGA::GUIEventAdapter::KEY_Shift_L:
            is_Shift_L = false;
            break;
        case osgGA::GUIEventAdapter::KEY_Shift_R:
            is_Shift_R = false;
            break;
        case osgGA::GUIEventAdapter::KEY_Control_L:
            is_Ctrl_L = false;
            break;
        case osgGA::GUIEventAdapter::KEY_Control_R:
            is_Ctrl_R = false;
            break;
        case osgGA::GUIEventAdapter::KEY_Alt_L:
            is_Alt_L = false;
            break;
        case osgGA::GUIEventAdapter::KEY_Alt_R:
            is_Alt_R = false;
            break;
        default: break;
        }
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
            OSG_FATAL << "Vehicle model from " << cfg_dir << "/" << cfg_file << " is't loaded" << std::endl;
            vehicle_exterior_t vehicle_ext = vehicle_exterior_t();
            vehicles_ext.push_back(vehicle_ext);
            OSG_FATAL << "Vehicle " << i + 1 << " / " << count << " added with empty model" << std::endl;
            continue;
        }
        OSG_FATAL << "Loaded vehicle model from " << cfg_dir << "/" << cfg_file << std::endl;

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
        //loadAnimations(cfg_dir, cfg_file, cabine.get(), *vehicle_ext.anims);
        loadSounds(cfg_dir, cfg_file, vehicle_ext.sounds_id);

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

        vehicles_ext[i].right = vehicles_ext[i].orth ^ vehicles_ext[i].up;
        //vehicles_ext[i].right.normalize();

        vehicles_ext[i].attitude = osg::Vec3(
            asinf(vehicles_ext[i].orth.z()),
            0.0f,
            (vehicles_ext[i].orth.x() > 0.0) ? acos(vehicles_ext[i].orth.y()) : - acos(vehicles_ext[i].orth.y()) );

        vehicles_ext[i].orientation = update_data[new_data].vehicles[i].orientation;

        // Apply vehicle body matrix transform
        osg::Matrix  matrix;
        matrix *= osg::Matrixf::rotate(vehicles_ext[i].attitude.x(), osg::Vec3(1.0f, 0.0f, 0.0f));
        matrix *= osg::Matrixf::rotate(-vehicles_ext[i].attitude.z(), osg::Vec3(0.0f, 0.0f, 1.0f));
        matrix *= osg::Matrixf::translate(vehicles_ext[i].position);

        vehicles_ext[i].transform->setMatrix(matrix);

        // Model animations update
        for (auto it = vehicles_ext[i].anims->begin(); it != vehicles_ext[i].anims->end(); ++it)
        {
            ProcAnimation *animation = it.value();
            animation->setPosition(update_data[new_data].vehicles[i].analogSignal[animation->getSignalID()]);
        }

        // Sounds update
        float dt = sim_data[new_data].time - sim_data[old_data].time;
        osg::Vec3 velocity = osg::Vec3( (sim_data[new_data].vehicles[i].position_x - sim_data[old_data].vehicles[i].position_x) / dt,
                                        (sim_data[new_data].vehicles[i].position_y - sim_data[old_data].vehicles[i].position_y) / dt,
                                        (sim_data[new_data].vehicles[i].position_z - sim_data[old_data].vehicles[i].position_z) / dt  );

        for (auto sound_id : vehicles_ext[i].sounds_id)
        {
            osg::Vec3 pos = vehicles_ext[i].position +
                            vehicles_ext[i].right * sound_manager->getLocalPositionX(sound_id) +
                            vehicles_ext[i].orth * sound_manager->getLocalPositionY(sound_id) +
                            vehicles_ext[i].up * sound_manager->getLocalPositionZ(sound_id);
            sound_manager->setPosition(sound_id, pos.x(), pos.y(), pos.z());
            sound_manager->setVelocity(sound_id, velocity.x(), velocity.y(), velocity.z());

            sound_manager->setSoundSignal(sound_id, update_data[new_data].vehicles[i].analogSignal[sound_manager->getSignalID(sound_id)]);
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

    double prev_time = 0.0;
    if (new_data != -1)
        prev_time = update_data[new_data].time;

    if (memory_sim_update.lock())
    {
        simulator_update_t *sd = static_cast<simulator_update_t *>(memory_sim_update.data());

        if ( (sd == nullptr) || (sd->time <= prev_time) )
        {
            memory_sim_update.unlock();
            return;
        }

        if (old_data == -1)
        {
            if (new_data == -1)
            {
                // Первое получение данных
                memcpy(update_data.data(), sd, sizeof (simulator_update_t));
                new_data = 0;
            }
            else
            {
                // Второе получение данных
                memcpy(update_data.data() + 1, sd, sizeof (simulator_update_t));
                old_data = 0;
                new_data = 1;
            }
        }
        else
        {
            // Обновление данных по очереди
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

        ref_time = 0;

        // Update debug string
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

        emit setStatusBar(hud_text);

        emit sendControlledState(controlled_vehicle == cur_vehicle);
    }
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
void TrainExteriorHandler::moveCamera(osgViewer::Viewer *viewer, float delta_time)
{
    camera_position_t cp;
    cp.position = vehicles_ext[static_cast<size_t>(cur_vehicle)].position;
    cp.attitude = vehicles_ext[static_cast<size_t>(cur_vehicle)].attitude;
    cp.driver_pos = vehicles_ext[static_cast<size_t>(cur_vehicle)].driver_pos;
    cp.is_orient_bwd = (vehicles_ext[static_cast<size_t>(cur_vehicle)].orientation < 0);

    cp.attitude.x() = - osg::PIf / 2.0f - cp.attitude.x() * settings.direction;

    cp.viewer_pos = vehicles_ext[static_cast<size_t>(cur_vehicle)].position
        + vehicles_ext[static_cast<size_t>(cur_vehicle)].orth * settings.direction * settings.stat_cam_shift;

    cp.view_basis.front = vehicles_ext[static_cast<size_t>(cur_vehicle)].orth;
    cp.view_basis.right = vehicles_ext[static_cast<size_t>(cur_vehicle)].right;

    emit sendCameraPosition(cp);

    // Move sound Listener
    osg::Vec3f tmp_eye, tmp_center, tmp_up;
    viewer->getCamera()->getViewMatrixAsLookAt(tmp_eye, tmp_center, tmp_up);

    osg::Vec3f pos = tmp_eye;
    osg::Vec3f velocity = (pos - prev_camera_pos) / delta_time;
    osg::Vec3f front = tmp_center - tmp_eye;
    front.normalize();
    osg::Vec3f up = tmp_up;

    sound_manager->setListenerPosition(pos.x(), pos.y(), pos.z());
    sound_manager->setListenerVelocity(velocity.x(), velocity.y(), velocity.z());
    sound_manager->setListenerOrientation(front.x(), front.y(), front.z(), up.x(), up.y(), up.z());
    prev_camera_pos = pos;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TrainExteriorHandler::loadSounds(const std::string &configDir,
                                          const std::string &configName,
                                          std::vector<size_t> &sounds_id)
{
    // Default name of animations configs directory is name of vehicle's config
    std::string sounds_config_dir = configName;

    FileSystem &fs = FileSystem::getInstance();
    std::string relative_cfg_path = configDir + fs.separator() + configName + ".xml";
    std::string cfg_path = fs.combinePath(fs.getVehiclesDir(), relative_cfg_path);

    // Load config file
    ConfigReader cfg(cfg_path);

    if (cfg.isOpenned())
    {
        std::string secName = "Vehicle";
        cfg.getValue(secName, "SoundDir", sounds_config_dir);
        OSG_FATAL << "Vehicle sounds are loaded from " << sounds_config_dir << std::endl;
    }

    sounds_id = sound_manager->loadVehicleSounds(QString(sounds_config_dir.c_str()));
}

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
        OSG_FATAL << "Vehicle model animations are loaded from " << anim_config_dir << std::endl;
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
            dc->display->setRouteDir(QString(settings.route_dir_full_path.c_str()));
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
