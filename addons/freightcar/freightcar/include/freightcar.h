#ifndef     FREIGHTCAR_H
#define     FREIGHTCAR_H

#include    "vehicle-api.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class FreightCar : public Vehicle
{
public:

    FreightCar();

    ~FreightCar();

    void initBrakeDevices(double p0, double pBP, double pFL);

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

    /// Авторежим
    BrakeAutoMode *automode;
    QString     automode_module;
    QString     automode_config;

    /// Тормозная рычажная передача
    BrakeMech   *brake_mech;
    QString     brake_mech_config;

    /// Признак включения регистрации
    bool is_Registrator_on;

    /// Регистратор параметров
    Registrator *registrator;

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

    void initRegistrator();

    void stepRegistrator(double t, double dt);

};

#endif // FREIGHTCAR_H
