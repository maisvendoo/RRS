#ifndef CHS2TBRAKEMECH_H
#define CHS2TBRAKEMECH_H

#include    "brake-mech.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class CHS2tBrakeMech : public BrakeMech
{
public:

    CHS2tBrakeMech(QObject *parent = Q_NULLPTR);

    ~CHS2tBrakeMech();

private:

    void load_config(CfgReader &cfg);
};

#endif // CHS2TBRAKEMECH_H
