#ifndef     TOPOLOGY_H
#define     TOPOLOGY_H

#include    <QObject>
#include    <QMap>

#include    "trajectory.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
typedef     QMap<QString, Trajectory *> traj_list_t;

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class Topology : public QObject
{
    Q_OBJECT

public:

    Topology(QObject *parent = Q_NULLPTR);

    virtual ~Topology();

    /// Загрузка топологии ж/д полигона
    bool load(QString route_dir);

protected:

    /// Контейнер данных по всем траекториям на полигоне
    traj_list_t     traj_list;

    /// Получить список имен всех имеющихся траекторий
    QStringList getTrajNamesList(QString route_dir);
};

#endif // TOPOLOGY_H
