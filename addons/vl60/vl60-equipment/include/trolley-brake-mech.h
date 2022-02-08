#ifndef     TROLLEY_BRAKE_MECH_H
#define     TROLLEY_BRAKE_MECH_H

#include    "brake-mech.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class TrolleyBrakeMech : public BrakeMech
{
public:

    TrolleyBrakeMech(QString cfg_path, QObject *parent = Q_NULLPTR);

    ~TrolleyBrakeMech();

private:

    void load_config(CfgReader &cfg);    
};

#endif // TROLLEY_BRAKE_MECH_H
