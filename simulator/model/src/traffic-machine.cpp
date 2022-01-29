#include    "traffic-machine.h"

#include    <QDir>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
TrafficMachine::TrafficMachine(QObject *parent) : QObject(parent)
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
TrafficMachine::~TrafficMachine()
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TrafficMachine::init(QString route_dir)
{
    QString path = QDir::toNativeSeparators(route_dir) +
            QDir::separator() + "stations.conf";

    QFile stations_file(path);

    if (!stations_file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        return;
    }

    stations_parse(stations_file);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TrafficMachine::stations_parse(QFile &stations_file)
{
    while (!stations_file.atEnd())
    {
        QByteArray line = stations_file.readLine();
        QStringList tokens = QString(line).remove('\n').split(';');

        if (tokens.size() < 3)
            continue;

        station_t station;
        station.begin_coord = tokens[0].toDouble();
        station.end_coord = tokens[1].toDouble();
        station.name = tokens[2];

        station.id = stations.size() + 1;

        stations.push_back(station);
    }
}
