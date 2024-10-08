#include <_mingw_mac.h>
#include    <traffic-lights-handler.h>
#include    <QBuffer>

#include    <iostream>

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
