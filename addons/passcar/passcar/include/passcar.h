//------------------------------------------------------------------------------
//
//		Passenger's car model for RRS
//		(c) maisvendoo, 16/02/2019
//
//------------------------------------------------------------------------------
#ifndef     PASSCAR_H
#define     PASSCAR_H

#include    <QMap>

#include    "vehicle-api.h"

#include    "registrator.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class PassCar : public Vehicle
{
public:

    PassCar();

    ~PassCar();

    void initBrakeDevices(double p0, double pTM, double pFL);

private:

    /// Тормозная магистраль
    Reservoir   *brakepipe;
    double      bp_leak;

    /// Воздухораспределитель
    AirDistributor  *air_dist;
    QString     air_dist_module;
    QString     air_dist_config;

    /// Электровоздухораспределитель
    ElectroAirDistributor   *electro_air_dist;
    QString     electro_air_dist_module;
    QString     electro_air_dist_config;

    /// Запасный резервуар
    Reservoir   *supply_reservoir;
    double      sr_volume;
    double      sr_leak;

    /// Концевой кран тормозной магистрали спереди
    PneumoAngleCock *anglecock_bp_fwd;
    /// Концевой кран тормозной магистрали сзади
    PneumoAngleCock *anglecock_bp_bwd;
    QString     anglecock_bp_config;

    /// Рукав тормозной магистрали спереди
    PneumoHoseEPB   *hose_bp_fwd;
    /// Рукав тормозной магистрали сзади
    PneumoHoseEPB   *hose_bp_bwd;
    QString     hose_bp_module;
    QString     hose_bp_config;

    /// Тормозная рычажная передача
    BrakeMech   *brake_mech;
    QString     brake_mech_config;

    /// Передаточное число редуктора в подвагонном электрогенераторе
    double ip;

    /// Признак включения регистрации
    bool is_Registarator_on;

    /// Регистратор параметров
    Registrator *registrator;

    QString     soundDir;

    QMap<int, QString> sounds;

    void initialization();

    /// Инициализация тормозного оборудования
    void initBrakesEquipment(QString modules_dir);

    /// Инициализация ЭПТ
    void initEPB(QString modules_dir);

    void step(double t, double dt);

    /// Моделирование тормозного оборудования
    void stepBrakesEquipment(double t, double dt);

    /// Моделирование ЭПТ
    void stepEPB(double t, double dt);

    /// Сигналы для анимации
    void stepSignalsOutput();

    /// Отладочная строка
    void stepDebugMsg(double t, double dt);

    void keyProcess();

    void loadConfig(QString cfg_path);

    void initSounds();

    void initRegistrator();

    void stepRegistrator(double t, double dt);

    void soundStep();

    void getSoundList();

    void playPasscarSound(QString sound_name);
};

#endif // PASSCAR_H
