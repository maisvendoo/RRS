#ifndef     CONNECTOR_H
#define     CONNECTOR_H

#include    <QObject>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class Connector : public QObject
{
public:

    Connector(QObject *parent);

    virtual ~Connector();

protected:
};

#endif
