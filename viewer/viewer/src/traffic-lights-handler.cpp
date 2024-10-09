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

    std::string path = fs.combinePath(settings.route_dir_name, "topology");
    path = fs.combinePath(path, "models-config.xml");

    ConfigReader cfg_reader;
    std::string models_dir;
    if (cfg_reader.load(path))
    {
        cfg_reader.getValue("Models", "SignalModelsDir", models_dir);
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

        osg::ref_ptr<osg::PagedLOD> pagedLOD = new osg::PagedLOD;
        pagedLOD->addDescription(model_base_name.toStdString());
        pagedLOD->setFileName(0, fullPath.toStdString());
        pagedLOD->setRange(0, 0.0f, settings.view_distance);
        pagedLOD->setRangeMode(osg::LOD::RangeMode::DISTANCE_FROM_EYE_POINT);

        signal_nodes.insert(model_base_name, pagedLOD);
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TrafficLightsHandler::load_signal_models(const settings_t &settings)
{
    create_pagedLODs(settings);

    for (auto tl = traffic_lights.begin(); tl != traffic_lights.end(); ++tl)
    {
        if (signal_nodes.value(tl.value()->getModelName(), nullptr).valid())
        {
            osg::ref_ptr<osg::MatrixTransform> transform = new osg::MatrixTransform;

            osg::Matrixd m1 = osg::Matrixd::translate(tl.value()->getPosition());

            osg::Vec3d o = tl.value()->getOrth();
            osg::Vec3d r = tl.value()->getRight();
            osg::Vec3d u = tl.value()->getUp();

            osg::Matrixd m2(r.x(), o.x(), u.y(), 0,
                            r.y(), o.y(), u.y(), 0,
                            r.z(), o.z(), u.z(), 0,
                            0, 0, 0, 1);

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
