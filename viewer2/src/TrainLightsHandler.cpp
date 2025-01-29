#include "TrainLightsHandler.h"
#include "Logger.h"
#include "TrafficLight.h"
#include <cstdint>
#include <qbuffer.h>
#include <qflags.h>

void TrainLightsHandler::deserialize(QByteArray& data)
{
    QBuffer buff(&data);
    buff.open(QIODevice::ReadOnly);
    QDataStream stream(&buff);

    std::uint32_t line_signals_size = 0;
    stream >> line_signals_size;

    LOG_INFO("Line signals: %u", line_signals_size);

    // Очищаем список сигналов
    traffic_lights_fwd.clear();
    traffic_lights_bwd.clear();

    for (std::uint32_t i = 0; i < line_signals_size; ++i)
    {
        QByteArray tmp_data;
        stream >> tmp_data;

        TrafficLight* tl = new TrafficLight;
        tl->deserialize(tmp_data);

        if (tl->getConnectorName().isEmpty())
        {
            continue;
        }


    }
}

void TrainLightsHandler::printSignalInfo(TrafficLight* tl)
{
    /*
    QString msg = QString("Signal at connector %1 is initialized. Letter: %2 | position: {%3, %4, %5} | direction: %6 {%7, %8, %9}")
                      .arg(tl->getConnectorName())
                      .arg(tl->getLetter())
                      .arg(tl->getPosition().x(), 8, 'f', 1)
                      .arg(tl->getPosition().y(), 8, 'f', 1)
                      .arg(tl->getPosition().z(), 8, 'f', 1)
                      .arg(tl->getSignalDirection() == -1 ? "BWD" : "FWD")
                      .arg(tl->getOrth().x(), 6, 'f', 3)
                      .arg(tl->getOrth().y(), 6, 'f', 3)
                      .arg(tl->getOrth().z(), 6, 'f', 3);*/

    LOG_INFO(
        "Signal at connector %s is initialized. Letter: %s | position: {%8.1f, %8.1f, %8.1f} | direction: %s {%6.3f %6.3f %6.3f}",
        tl->getConnectorName().toStdString().c_str(),
        tl->getLetter().toStdString().c_str(),
        tl->getPosition().x,
        tl->getPosition().y,
        tl->getPosition().z,
        (tl->getSignalDirection() == -1) ? "BWD" : "FDW",
        tl->getOrth().x,
        tl->getOrth().y,
        tl->getOrth().z
    );
}
