#ifndef     TRAFFIC_MACHINE_H
#define     TRAFFIC_MACHINE_H

#include    <QObject>
#include    <QFile>

#include    "traffic-train.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
struct station_t
{
    QString     name;
    int         id;
    double      begin_coord;
    double      end_coord;

    station_t()
        : name("")
        , id(0)
        , begin_coord(0.0)
        , end_coord(0.0)
    {

    }
};

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
struct TrafficMachine : public QObject
{
public:

    TrafficMachine(QObject *parent = Q_NULLPTR);

    ~TrafficMachine();

    void init(QString route_dir);

private:

    /// Массив станций на участке
    std::vector<station_t>   stations;

    /// Поезда, движущиеся по участку
    std::vector<TrafficTrain *> trains;

    /// Разбор файла с данными о станциях
    void stations_parse(QFile &stations_file);
};

#endif // TRAFFIC_MACHINE_H
