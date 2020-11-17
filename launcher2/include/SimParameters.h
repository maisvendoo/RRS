#ifndef SIMPARAMETERS_H
#define SIMPARAMETERS_H

#include <QObject>

class SimParameters : public QObject
{
    Q_OBJECT
public:
    explicit SimParameters(QObject* parent = nullptr);
    Q_INVOKABLE void setRoute(const QString& route);
    Q_INVOKABLE void setTrain(const QString& train);
    Q_INVOKABLE void setOrdinate(const double& ordinate);
    Q_INVOKABLE void setDirection(const bool& direction);
    Q_INVOKABLE void setWaypoint(const QString& waypoint);


    Q_INVOKABLE QString getRoute() const;
    Q_INVOKABLE QString getTrain() const;
    Q_INVOKABLE double  getOrdinate() const;
    Q_INVOKABLE bool    getDirection() const;
    Q_INVOKABLE QString getWaypoint() const;

private:
    QString m_route;
    QString m_train;
    QString m_waypoint;
    double  m_ordinate;
    bool    m_direction;

    Q_DISABLE_COPY(SimParameters)
};

#endif // SIMPARAMETERS_H
