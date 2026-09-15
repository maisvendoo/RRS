#ifndef     ELECTRO_VALVE_H
#define     ELECTRO_VALVE_H

#include    "relay.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class   ElectroValve : public Relay
{
public:

    ElectroValve(QObject *parent = Q_NULLPTR);

    virtual ~ElectroValve();

    bool getState() const;

private:


};

#endif      // ELECTROVALVE_H
