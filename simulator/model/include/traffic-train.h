#ifndef     TRAFFIC_TRAIN_H
#define     TRAFFIC_TRAIN_H

#include    <QObject>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
struct waypoint_t
{
    int station_id;
    double dep_time;
    double arr_time;
    double ordinate;

    waypoint_t()
        : station_id(0)
        , dep_time(0)
        , arr_time(0)
        , ordinate(0)
    {

    }
};

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class TrafficTrain : public QObject
{
public:

    TrafficTrain(QString timetable_file, QObject *parent = Q_NULLPTR);

    ~TrafficTrain();

private:

    std::vector<waypoint_t> waypoints;
};

#endif // TRAFFIC_TRAIN_H
