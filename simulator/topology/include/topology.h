#ifndef     TOPOLOGY_H
#define     TOPOLOGY_H

#include    <QObject>

#include    <topology-export.h>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class TOPOLOGY_EXPORT Topology : public QObject
{
public:

    Topology(QObject *parent = Q_NULLPTR);

    ~Topology();


private:


};

#endif
