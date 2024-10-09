#include    <traffic-lights-handler.h>
#include    <QBuffer>

#include    <iostream>
#include    <QDir>
#include    <QDirIterator>
#include    <QFile>
#include    <QFileInfo>
#include    <filesystem.h>
#include    <config-reader.h>
#include    <osgDB/ReadFile>
#include    <osg/MatrixTransform>
#include    <osg/BlendFunc>
#include    <osg/AlphaFunc>
#include    <osg/PolygonMode>
#include    <osg/Depth>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
TrafficLightsHandler::TrafficLightsHandler(QObject *parent)
    : QObject(parent)
    , osgGA::GUIEventHandler()
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
TrafficLightsHandler::~TrafficLightsHandler()
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool TrafficLightsHandler::handle(const osgGA::GUIEventAdapter &ea,
                                  osgGA::GUIActionAdapter &aa)
{
    switch (ea.getEventType())
    {
    case osgGA::GUIEventAdapter::FRAME:
        {
            for (auto tl = traffic_lights.begin(); tl != traffic_lights.end(); ++tl)
            {
                TrafficLight *traffic_light = tl.value();
                traffic_light->update();
            }

            break;
        }
    default: break;
    }

    return false;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TrafficLightsHandler::deserialize(QByteArray &data)
{
    QBuffer buff(&data);
    buff.open(QIODevice::ReadOnly);
    QDataStream stream(&buff);

    size_t data_size = 0;
    stream >> data_size;

    std::cout << "Line signals : " << data_size << std::endl;

    // Очищаем список сигналов
    traffic_lights.clear();

    for (size_t i = 0; i < data_size; ++i)
    {
        QByteArray tmp_data;
        stream >> tmp_data;

        TrafficLight *tl = new TrafficLight;
        tl->deserialize(tmp_data);

        if (tl->getConnectorName().isEmpty())
        {
            continue;
        }

        printSignalInfo(tl);

        traffic_lights.insert(tl->getConnectorName(), tl);
    }

    stream >> data_size;

    std::cout << "Enter signals : " << data_size << std::endl;

    for (size_t i = 0; i < data_size; ++i)
    {
        QByteArray tmp_data;
        stream >> tmp_data;

        TrafficLight *tl = new TrafficLight;
        tl->deserialize(tmp_data);

        if (tl->getConnectorName().isEmpty())
        {
            continue;
        }

        printSignalInfo(tl);

        traffic_lights.insert(tl->getConnectorName(), tl);
    }

    stream >> data_size;

    std::cout << "Exit signals : " << data_size << std::endl;

    for (size_t i = 0; i < data_size; ++i)
    {
        QByteArray tmp_data;
        stream >> tmp_data;

        TrafficLight *tl = new TrafficLight;
        tl->deserialize(tmp_data);

        if (tl->getConnectorName().isEmpty())
        {
            continue;
        }

        printSignalInfo(tl);

        traffic_lights.insert(tl->getConnectorName(), tl);
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TrafficLightsHandler::create_pagedLODs(const settings_t &settings)
{
    FileSystem &fs = FileSystem::getInstance();

    std::string path = fs.combinePath(settings.route_dir_full_path, "topology");
    path = fs.combinePath(path, "models-config.xml");

    ConfigReader cfg_reader;    
    if (cfg_reader.load(path))
    {
        cfg_reader.getValue("Models", "SignalModelsDir", models_dir);
        cfg_reader.getValue("Models", "SignalAnimationsDir", animations_dir);
    }

    std::string models_path = fs.getDataDir();
    models_path = fs.combinePath(models_path, "models");
    models_path = fs.combinePath(models_path, models_dir);

    QDir models(QString(models_path.c_str()));
    QDirIterator models_files(models.path(), QStringList() << "*.osgt", QDir::NoDotAndDotDot | QDir::Files);

    while (models_files.hasNext())
    {
        QString fullPath = models_files.next();
        QFileInfo fileInfo(fullPath);

        QString model_base_name = fileInfo.baseName();

        /*osg::ref_ptr<osg::PagedLOD> pagedLOD = new osg::PagedLOD;
        pagedLOD->addDescription(model_base_name.toStdString());
        pagedLOD->setFileName(0, fullPath.toStdString());
        pagedLOD->setRange(0, 0.0f, settings.view_distance);
        pagedLOD->setRangeMode(osg::LOD::RangeMode::DISTANCE_FROM_EYE_POINT);

        signal_nodes.insert(model_base_name, pagedLOD);*/

        osg::ref_ptr<osg::Node> model = osgDB::readNodeFile(fullPath.toStdString());

        osg::StateSet *ss = model->getOrCreateStateSet();
        model->setDataVariance(osg::Object::DYNAMIC);

        // Set blend function for model
        osg::ref_ptr<osg::BlendFunc> blendFunc = new osg::BlendFunc(osg::BlendFunc::SRC_ALPHA,
                                                                    osg::BlendFunc::ONE_MINUS_SRC_ALPHA);
        ss->setAttributeAndModes(blendFunc.get());
        ss->setMode(GL_BLEND, osg::StateAttribute::ON);
        ss->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);

        // Set alpha function for model
        osg::ref_ptr<osg::AlphaFunc> alphaFunc = new osg::AlphaFunc(osg::AlphaFunc::GEQUAL, 0.6f);
        ss->setAttributeAndModes(alphaFunc.get());
        ss->setMode(GL_ALPHA_TEST, osg::StateAttribute::ON);

        ss->setAttributeAndModes(new osg::Depth(osg::Depth::LEQUAL, 0.0, 1.0));

        osg::ref_ptr<osg::PolygonMode> pm = new osg::PolygonMode;
        pm->setMode(osg::PolygonMode::FRONT, osg::PolygonMode::FILL);
        ss->setAttribute(pm.get());

        signal_nodes.insert(model_base_name, model);
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TrafficLightsHandler::slotUpdateSignal(QByteArray data)
{
    QBuffer buff(&data);
    buff.open(QIODevice::ReadOnly);
    QDataStream stream(&buff);

    QString conn_name = "";
    int signal_dir = 0;

    stream >> conn_name;
    stream >> signal_dir;

    if (conn_name.isEmpty())
    {
        return;
    }

    TrafficLight *tl = traffic_lights.value(conn_name, nullptr);

    if (tl == nullptr)
    {
        return;
    }

    tl->deserialize(data);
    tl->update();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TrafficLightsHandler::load_signal_models(const settings_t &settings)
{

    for (auto tl = traffic_lights.begin(); tl != traffic_lights.end(); ++tl)
    {
        if (signal_nodes.value(tl.value()->getModelName(), nullptr).valid())
        {
            osg::ref_ptr<osg::MatrixTransform> transform = new osg::MatrixTransform;

            osg::Matrixd m1 = osg::Matrixd::translate(tl.value()->getPosition());

            int sd = tl.value()->getSignalDirection();

            osg::Vec3d o = tl.value()->getOrth() * sd;
            osg::Vec3d r = tl.value()->getRight() * sd;
            osg::Vec3d u = tl.value()->getUp();


            osg::Matrixd m2(r.x(), o.x(), u.y(), 0,
                            r.y(), o.y(), u.y(), 0,
                            r.z(), o.z(), u.z(), 0,
                            0, 0, 0, 1);

            osg::Node *signal_node = signal_nodes.value(tl.value()->getModelName(), nullptr).get();
            TrafficLight *traffic_light = tl.value();
            traffic_light->setNode(signal_node);
            traffic_light->load_animations(animations_dir);
            //traffic_light->update();

            animation_mangers.push_back(new AnimationManager(traffic_light->getAnimationsListPtr()));

            transform->setMatrix(m2 * m1);
            transform->addChild(signal_nodes.value(tl.value()->getModelName(), nullptr).get());
            signals_group->addChild(transform);
        }
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TrafficLightsHandler::printSignalInfo(TrafficLight *tl)
{
    QString msg = QString("Signal in connector %1 is initialized. Letter: %2 | position: {%3, %4, %5}")
                      .arg(tl->getConnectorName())
                      .arg(tl->getLetter())
                      .arg(tl->getPosition().x())
                      .arg(tl->getPosition().y())
                      .arg(tl->getPosition().z());

    std::cout << msg.toStdString() << std::endl;
}
