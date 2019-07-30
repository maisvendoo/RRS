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
#include    "reservoir.h"
#include    "airdistributor.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class PassCarrige : public Vehicle
{
public:

    PassCarrige();

    ~PassCarrige();    

    void initBrakeDevices(double p0, double pTM, double pFL);

private:

    BrakeMech   *brake_mech;
    Reservoir   *supply_reservoir;

    QString     brake_mech_module;
    QString     brake_mech_config;

    AirDistributor *airdist;

    QString     airdist_module;
    QString     airdist_config;   

    void initialization();

    void step(double t, double dt);

    void stepSignalsOutput();

    void keyProcess();

    void loadConfig(QString cfg_path);
};

#endif // PASSCAR_H
