//------------------------------------------------------------------------------
//
//
//
//
//
//------------------------------------------------------------------------------
#ifndef     TEST_LOCO_H
#define     TEST_LOCO_H

#include    "vehicle.h"
#include    "brake-crane.h"
#include    "brake-mech.h"
#include    "physics.h"
#include    "controls.h"
#include    "reservoir.h"
#include    "airdistributor.h"
#include    "pneumo-relay.h"
#include    "switching-valve.h"
#include    "loco-crane.h"
#include    "automatic-train-stop.h"
#include    "traction-controller.h"
#include    "train-horn.h"


class TestLoco : public Vehicle
{
public:

    TestLoco();
    ~TestLoco();

    void initBrakeDevices(double p0, double pTM);

private:

    double tau;
    double delay;

    double traction_level;

    bool inc_loc;
    bool dec_loc;

    bool inc_crane_loc;
    bool dec_crane_loc;

    int crane_pos;
    int crane_step;
    double crane_motion;
    double crane_duration;

    double pz;
    bool inc_brake;
    bool dec_brake;

    QString brake_crane_module;
    QString brake_crane_config;

    BrakeCrane  *brake_crane;
    BrakeMech   *brake_mech;

    QString     brake_mech_module;
    QString     brake_mech_config;

    double charge_press;

    QString     loco_crane_module;
    QString     loco_crane_config;    

    double  loco_crane_pos;

    QString     autostop_module;
    QString     autostop_config;

    QString     traction_controller_module;
    QString     traction_controller_config;

    IncTractionTrigger incTracTrig;
    DecTractionTrigger decTracTrig;

    IncBrakeCrane incBrakeCrane;
    DecBrakeCrane decBrakeCrane;

    IncChargePress incChargePress;
    DecChargePress decChargePress;

    AirDistributor *airdist;

    QString     airdist_module;
    QString     airdist_config;

    Reservoir   *supply_reservoir;

    PneumoReley *repiter;

    SwitchingValve *zpk;

    LocoCrane *loco_crane;


    AutoTrainStop *autostop;

    TractionController *trac_controller;

    TrainHorn   *horn;

    void step(double t, double dt);

    double traction_char(double V);

    void initialization();

    void loadConfig(QString cfg_path);

    void keyProcess();        
};




#endif // TESTLOCO_H
