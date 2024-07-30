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

    /// Сцепка спереди
    Coupling *coupling_fwd;
    /// Сцепка сзади
    Coupling *coupling_bwd;
    QString coupling_module_name;
    QString coupling_config_name;

    /// Расцепной рычаг спереди
    OperatingRod *oper_rod_fwd;
    /// Расцепной рычаг сзади
    OperatingRod *oper_rod_bwd;

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
    bool is_Registrator_on;

    /// Регистратор параметров
    Registrator *registrator;

    QString     soundDir;

    QMap<int, QString> sounds;

    void initialization();

    /// Инициализация сцепных устройств
    void initCouplings(const QString &modules_dir, const QString &custom_cfg_dir);

    /// Инициализация тормозного оборудования
    void initBrakesEquipment(const QString &modules_dir, const QString &custom_cfg_dir);

    /// Инициализация ЭПТ
    void initEPB(const QString &modules_dir, const QString &custom_cfg_dir);

    /// Инициализация регистратора параметров в лог-файл
    void initRegistrator(const QString &modules_dir, const QString &custom_cfg_dir);

    /// Предварительные расчёты перед симуляцией
    void preStep(double t);

    /// Предварительный расчёт координат сцепных устройств
    void preStepCouplings(double t);

    /// Шаг моделирования
    void step(double t, double dt);

    /// Моделирование сцепных устройств
    void stepCouplings(double t, double dt);

    /// Моделирование тормозного оборудования
    void stepBrakesEquipment(double t, double dt);

    /// Моделирование ЭПТ
    void stepEPB(double t, double dt);

    /// Вывод параметров в лог-файл
    void stepRegistrator(double t, double dt);

    /// Сигналы для анимации
    void stepSignalsOutput();

    /// Сигналы для звуков
    void stepSoundsSignals(double t, double dt);

    /// Отладочная строка
    void stepDebugMsg(double t, double dt);

    void keyProcess();

    void loadConfig(QString cfg_path);
};

#endif // PASSCAR_H
