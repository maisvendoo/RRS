#ifndef     TOPOLOGY_H
#define     TOPOLOGY_H

#include    <QObject>

#include    <topology-export.h>

/*!
 * \class
 * \brief Класс, обеспечивающий расчет положения ПЕ на путевой структуре
 */
//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class TOPOLOGY_EXPORT Topology : public QObject
{
    Q_OBJECT

public:

    Topology(QObject *parent = Q_NULLPTR);

    ~Topology();


    bool init(QString route_dir);

    void step(double t, double dt);

private:


};

#endif
