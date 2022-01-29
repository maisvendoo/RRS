#include    "traffic-train.h"

#include    "CfgReader.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
TrafficTrain::TrafficTrain(QObject *parent)
    : QObject(parent)
    , stations(Q_NULLPTR)
    , is_ready(false)
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
TrafficTrain::~TrafficTrain()
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TrafficTrain::init(QString timetable_file)
{
    load_waypoints(timetable_file);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TrafficTrain::load_waypoints(QString timetable_file)
{
    CfgReader cfg;

    if (!cfg.load(timetable_file))
        return;

    QDomNode stationNode = cfg.getFirstSection("Station");

    while (!stationNode.isNull())
    {
        int id = 0;

        if (!cfg.getInt(stationNode, "id", id))
            continue;

        waypoint_t waypoint;
        waypoint.station_id = id;
        waypoint.ordinate = (stations->at(id-1).begin_coord + stations->at(id-1).end_coord) / 2.0;

        QString dep_time = "";

        if (!cfg.getString(stationNode, "DepTime", dep_time))
            continue;

        waypoint.dep_time = convertTime(dep_time);

        QString arr_time = "";

        if (!cfg.getString(stationNode, "ArrTime", arr_time))
            continue;

        waypoint.arr_time = convertTime(arr_time);

        waypoints.push_back(waypoint);

        stationNode = cfg.getNextSection();
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double TrafficTrain::convertTime(QString time)
{
    return 0;
}
