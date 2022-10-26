//------------------------------------------------------------------------------
//
//		Passagire's carrige model for RRS
//		(c) maisvendoo, 16/02/2019
//
//------------------------------------------------------------------------------
#ifndef     PASSCAR_H
#define     PASSCAR_H

#include    <QMap>

#include    "vehicle.h"
#include    "brake-mech.h"
#include    "reservoir.h"
#include    "airdistributor.h"
#include    "electro-airdistributor.h"
#include    "pneumo-anglecock.h"
#include    "registrator.h"

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

    /// Регистратор параметров в лог-файл
    Registrator *reg;

    /// Резервуар в качестве трубы тормозной магистрали
    Reservoir *brakepipe;

    /// Резервуар в качестве переднего рукава тормозной магистрали
    Reservoir *hose_tm_fwd;

    /// Резервуар в качестве заднего рукава тормозной магистрали
    Reservoir *hose_tm_bwd;

    /// Концевой кран переднего рукава тормозной магистрали
    PneumoAngleCock *anglecock_tm_fwd;

    /// Концевой кран заднего рукава тормозной магистрали
    PneumoAngleCock *anglecock_tm_bwd;

    BrakeMech   *brake_mech;
    Reservoir   *supply_reservoir;

    QString     brake_mech_module;
    QString     brake_mech_config;

    AirDistributor *airdist;

    QString     airdist_module;
    QString     airdist_config;

    ElectroAirDistributor *electroAirDist;

    QString     electro_airdist_module;
    QString     electro_airdist_config;

    QString     soundDir;

    QMap<int, QString> sounds;

    void initialization();

    /// Инициализация тормозной магистрали
    void initBrakepipe(QString module_path);

    void step(double t, double dt);

    /// Запись параметров в лог-файл
    void stepRegistrator(double t, double dt);

    /// Моделирование тормозной магистрали
    void stepBrakepipe(double t, double dt);

    void stepSignalsOutput();

    void keyProcess();

    void loadConfig(QString cfg_path);

    void initEPT();

    void initSounds();

    void soundStep();

    void getSoundList();

    void playPasscarSound(QString sound_name);

    void stepEPT(double t, double dt);
};

#endif // PASSCAR_H
