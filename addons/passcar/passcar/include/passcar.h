//------------------------------------------------------------------------------
//
//		Passagire's carrige model for RRS
//		(c) maisvendoo, 16/02/2019
//
//------------------------------------------------------------------------------
#ifndef     PASSCAR_H
#define     PASSCAR_H

#include    "vehicle.h"
#include    "brake-mech.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class PassCarrige : public Vehicle
{
public:

    PassCarrige();

    ~PassCarrige();    

private:

    BrakeMech   *brake_mech;

    QString     brake_mech_module;
    QString     brake_mech_config;

    double      pz;
    bool        inc_loc;
    bool        dec_loc;

    void initialization();

    void step(double t, double dt);

    void keyProcess();

    void loadConfig(QString cfg_path);
};

#endif // PASSCAR_H
