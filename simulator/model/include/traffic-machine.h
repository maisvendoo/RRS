#ifndef     TRAFFIC_MACHINE_H
#define     TRAFFIC_MACHINE_H

#include    <QObject>
#include    <QDir>
#include    <QFile>

#include    "traffic-train.h"
#include    "traffic-common-types.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
struct TrafficMachine : public QObject
{
public:

    TrafficMachine(QObject *parent = Q_NULLPTR);

    ~TrafficMachine();

    bool init(QString route_dir);

private:

    /// Массив станций на участке
    stations_list_t  stations;

    /// Поезда, движущиеся по участку
    std::vector<TrafficTrain *> trains;

    /// Разбор файла с данными о станциях
    void stations_parse(QFile &stations_file);

    /// Загрузка поездов, согласно графикам их движения
    /// в каталоге timetables с маршрутом
    void load_trains(QDir &timetables_dir);
};

#endif // TRAFFIC_MACHINE_H
