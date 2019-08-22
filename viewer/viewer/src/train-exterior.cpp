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

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
TrainExteriorHandler::TrainExteriorHandler(settings_t settings,
                                           MotionPath *routePath,
                                           const std::string &train_config)
    : QObject(Q_NULLPTR)
    , osgGA::GUIEventHandler ()
    , settings(settings)
    , cur_vehicle(0)
    , long_shift(0.0f)
    , height_shift(0.0f)
    , routePath(routePath)
    , trainExterior(new osg::Group)
    , ref_time(0.0)

{
    load(train_config);

    shared_memory.setKey("sim");

    if (!shared_memory.attach(QSharedMemory::ReadOnly))
    {
        OSG_FATAL << "Can't connect to shared memory" << std::endl;
    }

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

            moveTrain(ref_time, nd);

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
AnimationManager *TrainExteriorHandler::getAnimationManager()
{
    return animation_manager;
}

std::vector<AnimationManager *> TrainExteriorHandler::getAnimManagers()
{
    return anim_managers;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TrainExteriorHandler::keyboardHandler(int key)
{
    switch (key)
    {
    case osgGA::GUIEventAdapter::KEY_Page_Down:

        cur_vehicle++;

        if (cur_vehicle > static_cast<int>(vehicles_ext.size() - 1))
            cur_vehicle = 0;

        long_shift = 0.0f;

        break;

    case osgGA::GUIEventAdapter::KEY_Page_Up:

        cur_vehicle--;

        if (cur_vehicle < 0)
            cur_vehicle = static_cast<int>(vehicles_ext.size() - 1);

        long_shift = 0.0f;

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

        break;

    case osgGA::GUIEventAdapter::KEY_Delete:

        height_shift -= 0.05f;
        break;

    case osgGA::GUIEventAdapter::KEY_Insert:

        height_shift += 0.05f;
        break;
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TrainExteriorHandler::load(const std::string &train_config)
{
    // Check train config name
    if (train_config.empty())
    {
        OSG_FATAL << "Train config is't referenced" << std::endl;
        return;
    }

    // Loading train config XML-file
    FileSystem &fs = FileSystem::getInstance();
    std::string path = fs.combinePath(fs.getTrainsDir(), train_config + ".xml");

    ConfigReader cfg(path);

    if (!cfg.isOpenned())
    {
        OSG_FATAL << "Train's config file " << path << " is't opened" << std::endl;
        return;
    }

    osgDB::XmlNode *config_node = cfg.getConfigNode();

    if (config_node == nullptr)
    {
        OSG_FATAL << "There is no Config node in file " << path << std::endl;
        return;
    }

    // Parsing of train config file
    for (auto it = config_node->children.begin(); it != config_node->children.end(); ++it)
    {
        osgDB::XmlNode *child = *it;

        // Check that node is Vehicle node
        if (child->name == "Vehicle")
        {
            int count = 0;

            osgDB::XmlNode *count_node = cfg.findSection(child, "Count");

            if (count_node == nullptr)
            {
                OSG_FATAL << "Number of vehicles is't referenced" << std::endl;
                continue;
            }

            // Read vehicles number
            getValue(count_node->contents, count);

            std::string module_config_name = "";

            osgDB::XmlNode *module_config_node = cfg.findSection(child, "ModuleConfig");

            if (module_config_node == nullptr)
            {
                OSG_FATAL << "Vehicle module config is't referenced" << std::endl;
                continue;
            }

            // Load vehicle body model
            getValue(module_config_node->contents, module_config_name);            

            for (int i = 0; i < count; ++i)
            {
                osg::ref_ptr<osg::Group> vehicle_model = loadVehicle(module_config_name);

                if (!vehicle_model.valid())
                {
                    OSG_FATAL << "Vehicle model " << module_config_name << " is't loaded" << std::endl;
                    continue;
                }

                // Load cabine model
                osg::ref_ptr<osg::Node> cabine;
                loadCabine(vehicle_model.get(), module_config_name, cabine);

                float length = getLength(module_config_name);

                osg::Vec3 driver_pos = getDirverPosition(module_config_name);

                vehicle_exterior_t vehicle_ext;
                vehicle_ext.transform = new osg::MatrixTransform;
                vehicle_ext.transform->addChild(vehicle_model.get());                
                vehicle_ext.length = length;
                vehicle_ext.cabine = cabine;
                vehicle_ext.driver_pos = driver_pos;

                vehicle_ext.anims = new animations_t();

                loadModelAnimations(module_config_name, vehicle_model.get(), *vehicle_ext.anims);
                loadAnimations(module_config_name, vehicle_model.get(), *vehicle_ext.anims);
                loadAnimations(module_config_name, cabine.get(), *vehicle_ext.anims);

                anim_managers.push_back(new AnimationManager(vehicle_ext.anims));

                vehicles_ext.push_back(vehicle_ext);
                trainExterior->addChild(vehicle_ext.transform.get());
            }
        }
    }

    //animation_manager = new AnimationManager(&animations);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TrainExteriorHandler::moveTrain(double ref_time, const network_data_t &nd)
{
    if (nd.sd.size() < 2)
        return;    

    // Time to relative units conversion
    float Delta_t = static_cast<float>(settings.request_interval) / 1000.0f;

    float t = static_cast<float>(ref_time) / Delta_t;

    for (size_t i = 0; i < vehicles_ext.size(); i++)
    {
        // Interframe railway coordinate and wheels angle linear interpolation
        float coord = (1.0f - t) * nd.sd.front().te[i].coord + nd.sd.back().te[i].coord * t;

        // Vehicle cartesian position and attitude calculation
        vehicles_ext[i].position = routePath->getPosition(coord, vehicles_ext[i].attitude);

        if (settings.direction == -1)
            vehicles_ext[i].attitude.z() = osg::PIf + vehicles_ext[i].attitude.z();

        // Store current railway coordinate and wheels angle
        vehicles_ext[i].coord = coord;
        //vehicles_ext[i].wheel_angle = angle;

        recalcAttitude(i);

        // Apply vehicle body matrix transform
        osg::Matrix  matrix;
        matrix *= osg::Matrixf::rotate(settings.direction * vehicles_ext[i].attitude.x(), osg::Vec3(1.0f, 0.0f, 0.0f));
        matrix *= osg::Matrixf::rotate(-vehicles_ext[i].attitude.z(), osg::Vec3(0.0f, 0.0f, 1.0f));
        matrix *= osg::Matrixf::translate(vehicles_ext[i].position);

        vehicles_ext[i].transform->setMatrix(matrix);

        for (auto it = vehicles_ext[i].anims->begin(); it != vehicles_ext[i].anims->end(); ++it)
        {
            ProcAnimation *animation = it.value();
            animation->setPosition(nd.sd.back().te[i].analogSignal[animation->getSignalID()]);
        }
    }

    // Set parameters for animations
    /*for (auto it = animations.begin(); it != animations.end(); ++it)
    {
        ProcAnimation *animation = it.value();
        animation->setPosition(nd.sd.back().te[static_cast<size_t>(cur_vehicle)].analogSignal[animation->getSignalID()]);
    }*/
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TrainExteriorHandler::processSharedData(double &ref_time)
{    
    if (ref_time < static_cast<double>(settings.request_interval) / 1000.0)
        return;

    ref_time = 0;

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

    cp.attitude.x() = - osg::PIf / 2.0f - cp.attitude.x() * settings.direction;

    float viewer_coord = vehicles_ext[static_cast<size_t>(cur_vehicle)].coord +
            settings.direction * settings.stat_cam_shift;

    cp.viewer_pos = routePath->getPosition(viewer_coord, cp.view_basis);

    cp.view_basis.right = cp.view_basis.right * settings.direction;

    emit sendCameraPosition(cp);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TrainExteriorHandler::recalcAttitude(size_t i)
{
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
    osg::Vec3 tail_orth = osg::Vec3(-cosf(pitch) * sinf(yaw), -cosf(pitch) * cosf(yaw), -sinf(pitch));

    // Calculate bacward coupling point of previos vehicle
    osg::Vec3 tail_dir = tail_orth * (curr.length / 2.0f);

    // Calculate forward coupling of current vehicle orth
    osg::Vec3 a = prev.position + tail_dir;
    osg::Vec3 forward = a - curr.position;
    osg::Vec3 f_orth = forward * (1 / forward.length());

    // Calculate new attitude of current vehicle
    float y_new = arg(f_orth.y(), f_orth.x());
    float y_old = curr.attitude.z();

    // "Smoothing" of vehicle oscillations
    vehicles_ext[i].attitude.z() = (y_new + y_old) / 2.0f;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TrainExteriorHandler::loadAnimations(const std::string vehicle_name,
                                          osg::Node *cabine,
                                          animations_t &animations)
{
    if (cabine == nullptr)
        return;

    AnimTransformVisitor atv(&animations, vehicle_name);
    atv.setTraversalMode(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN);

    cabine->accept(atv);    
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TrainExteriorHandler::loadModelAnimations(const std::string vehicle_name,
                                               osg::Node *model,
                                               animations_t &animations)
{
    FileSystem &fs = FileSystem::getInstance();

    std::string animations_dir = fs.combinePath(fs.getDataDir(), "animations");
    animations_dir = fs.combinePath(animations_dir, vehicle_name);

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
