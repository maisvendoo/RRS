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
    , start_time(0.0)
{
    load(train_config);

    shared_memory.setKey("sim");

    if (!shared_memory.attach(QSharedMemory::ReadOnly))
    {
        OSG_FATAL << "Can't connect to shared memory" << std::endl;
    }
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

            double time = viewer->getFrameStamp()->getReferenceTime();
            delta_time = time - start_time;
            ref_time += delta_time;
            start_time = time;

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
        OSG_FATAL << "There is no Cnfig node in file " << path << std::endl;
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

            osg::ref_ptr<osg::Group> vehicle_model = loadVehicle(module_config_name);

            if (!vehicle_model.valid())
            {
                OSG_FATAL << "Vehicle model " << module_config_name << " is't loaded" << std::endl;
                continue;
            }

            // Load wheels model
            osg::ref_ptr<osg::MatrixTransform> wheel_model = loadWheels(module_config_name);

            if (!wheel_model.valid())
            {
                OSG_FATAL << "Wheels model is't loaded" << std::endl;
                continue;
            }

            // Set wheels for each axis
            setAxis(vehicle_model.get(), wheel_model.get(), module_config_name);

            // Load cabine model
            osg::ref_ptr<osg::Node> cabine;
            loadCabine(vehicle_model.get(), module_config_name, cabine);

            float length = getLength(module_config_name);

            osg::Vec3 driver_pos = getDirverPosition(module_config_name);

            loadAnimations(module_config_name, cabine.get(), animations);

            for (int i = 0; i < count; ++i)
            {
                vehicle_exterior_t vehicle_ext;
                vehicle_ext.transform = new osg::MatrixTransform;
                vehicle_ext.transform->addChild(vehicle_model.get());
                vehicle_ext.wheel_rotation = wheel_model.get();
                vehicle_ext.length = length;
                vehicle_ext.cabine = cabine;
                vehicle_ext.driver_pos = driver_pos;

                vehicles_ext.push_back(vehicle_ext);
                trainExterior->addChild(vehicle_ext.transform.get());
            }
        }
    }

    animation_manager = new AnimationManager(&animations);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TrainExteriorHandler::moveTrain(double ref_time, const network_data_t &nd)
{
    if (nd.sd.empty())
        return;

    // Time to relative units conversion
    float t = static_cast<float>(ref_time) / nd.delta_time;

    for (size_t i = 0; i < vehicles_ext.size(); i++)
    {
        // Interframe railway coordinate and wheels angle linear interpolation
        float coord = (1.0f - t) * nd.sd.front().te[i].coord + nd.sd.back().te[i].coord * t;
        float angle = (1.0f - t) * nd.sd.front().te[i].angle + nd.sd.back().te[i].angle * t;

        // Vehicle cartesian position and attitude calculation
        vehicles_ext[i].position = routePath->getPosition(coord, vehicles_ext[i].attitude);

        if (settings.direction == -1)
            vehicles_ext[i].attitude.z() = osg::PIf + vehicles_ext[i].attitude.z();

        // Store current railway coordinate and wheels angle
        vehicles_ext[i].coord = coord;
        vehicles_ext[i].wheel_angle = angle;

        recalcAttitude(i);

        // Apply vehicle body matrix transform
        osg::Matrix  matrix;
        matrix *= osg::Matrix::rotate(static_cast<double>(settings.direction * vehicles_ext[i].attitude.x()), osg::Vec3(1.0f, 0.0f, 0.0f));
        matrix *= osg::Matrix::rotate(static_cast<double>(-vehicles_ext[i].attitude.z()), osg::Vec3(0.0f, 0.0f, 1.0f));
        matrix *= osg::Matrix::translate(vehicles_ext[i].position);
        vehicles_ext[i].transform->setMatrix(matrix);

        // Apply wheel axis rotation
        osg::Matrix rotMat = osg::Matrix::rotate(static_cast<double>(- settings.direction * vehicles_ext[i].wheel_angle), osg::Vec3(1.0f, 0.0f, 0.0f));

        if (vehicles_ext[i].wheel_rotation != nullptr)
            vehicles_ext[i].wheel_rotation->setMatrix(rotMat);
    }

    // Set parameters for animations
    for (auto it = animations.begin(); it != animations.end(); ++it)
    {
        ProcAnimation *animation = it.value();
        animation->setPosition(nd.sd.back().te[static_cast<size_t>(cur_vehicle)].analogSignal[animation->getSignalID()]);
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

        nd.sd.push(server_data);

        if (nd.sd.size() > 2)
        {
            nd.sd.pop();
            nd.delta_time = nd.sd.back().time - nd.sd.front().time;
        }

        QString msg = QString("ПЕ #%1: delta_time: %2 nd.delta_time: %3").arg(cur_vehicle).arg(delta_time, 6, 'f', 4).arg(nd.delta_time, 6, 'f', 4);
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

    cp.attitude.x() = -osg::PIf / 2.0f - cp.attitude.x();

    float viewer_coord = vehicles_ext[static_cast<size_t>(cur_vehicle)].coord + settings.stat_cam_shift;
    cp.viewer_pos = routePath->getPosition(viewer_coord, cp.view_basis);

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
