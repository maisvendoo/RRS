#include "SimParameters.h"

SimParameters::SimParameters(QObject *parent)
    : QObject(parent)
    , m_route()
    , m_train()
    , m_waypoint()
    , m_ordinate()
    , m_direction(true)
{

}

void SimParameters::setRoute(const QString &route)
{
    m_route = route;
}

void SimParameters::setTrain(const QString &train)
{
    m_train = train;
}

void SimParameters::setOrdinate(const double &ordinate)
{
    m_ordinate = ordinate;
}

void SimParameters::setDirection(const bool &direction)
{
    m_direction = direction;
}

void SimParameters::setWaypoint(const QString &waypoint)
{
    m_waypoint = waypoint;
}

QString SimParameters::getRoute() const
{
    return m_route;
}

QString SimParameters::getTrain() const
{
    return m_train;
}

double SimParameters::getOrdinate() const
{
    return m_ordinate;
}

bool SimParameters::getDirection() const
{
    return m_direction;
}

QString SimParameters::getWaypoint() const
{
    return m_waypoint;
}
