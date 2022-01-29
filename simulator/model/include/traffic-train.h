#ifndef     TRAFFIC_TRAIN_H
#define     TRAFFIC_TRAIN_H

#include    <QObject>

#include    "traffic-common-types.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class TrafficTrain : public QObject
{
public:

    TrafficTrain(QObject *parent = Q_NULLPTR);

    ~TrafficTrain();

    void setStationsList(stations_list_t *stations)
    {
        this->stations = stations;
    }

    void init(QString timetable_file);

    bool isReady() const { return is_ready; }

private:

    stations_list_t *stations;

    bool is_ready;

    std::vector<waypoint_t> waypoints;

    void load_waypoints(QString timetable_file);

    double convertTime(QString time);
};

#endif // TRAFFIC_TRAIN_H
