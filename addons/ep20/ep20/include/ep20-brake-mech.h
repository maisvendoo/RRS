#ifndef EP20_BRAKE_MECH_H
#define EP20_BRAKE_MECH_H

#include    "brake-mech.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class EP20BrakeMech : public BrakeMech
{
public:

    EP20BrakeMech(QObject *parent = Q_NULLPTR);

    ~EP20BrakeMech();

private:

    void load_config(CfgReader &cfg);
};

#endif // EP20_BRAKE_MECH_H
