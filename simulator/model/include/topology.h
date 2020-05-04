#ifndef     TOPOLOGY_H
#define     TOPOLOGY_H

#include    <QObject>

#include    "trajectory.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class Topology : public QObject
{
    Q_OBJECT

public:

    Topology(QObject *parent = Q_NULLPTR);

    virtual ~Topology();

protected:


};

#endif // TOPOLOGY_H
