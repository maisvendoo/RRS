//------------------------------------------------------------------------------
//
//		Passenger's car model for RRS
//		(c) maisvendoo, 16/02/2019
//
//------------------------------------------------------------------------------
#ifndef     PASSCAR_H
#define     PASSCAR_H

#include "vehicle.h"
#include "trigger-control.h"

#include <QString>

class AirDistributor;
class BrakeMech;
class Coupling;
class ElectroAirDistributor;
class OperatingRod;
class PneumoAngleCock;
class PneumoHoseEPB;
class Registrator;
class Reservoir;
class BrakeShoes;

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class PassCar : public Vehicle
{
public:

    PassCar();

    ~PassCar();

    void initBrakeDevices(double p0, double pTM, double pFL) override;

private:

    /// Включение трёх красных огней на передней торцевой стенке
    TriggerControl red_lamps_end_of_train_fwd;
    /// Включение трёх красных огней на задней торцевой стенке
    TriggerControl red_lamps_end_of_train_bwd;

    /// Сцепка спереди
    Coupling *coupling_fwd = nullptr;
    QString coupling_module_name = "sa3";
    /// Сцепка сзади
    Coupling *coupling_bwd = nullptr;
    QString coupling_config_name = "sa3";

    /// Расцепной рычаг спереди
    OperatingRod *oper_rod_fwd = nullptr;
    /// Расцепной рычаг сзади
    OperatingRod *oper_rod_bwd = nullptr;

    /// Тормозная магистраль
    Reservoir   *brakepipe = nullptr;
    double      bp_leak = 0.0;

    /// Воздухораспределитель
    AirDistributor  *air_dist = nullptr;
    QString     air_dist_module = "vr292";
    QString     air_dist_config = "vr292";

    /// Электровоздухораспределитель
    ElectroAirDistributor   *electro_air_dist = nullptr;
    QString     electro_air_dist_module = "";
    QString     electro_air_dist_config = "";

    /// Запасный резервуар
    Reservoir   *supply_reservoir = nullptr;
    double      sr_volume = 0.078;
    double      sr_leak = 0.0;

    /// Концевой кран тормозной магистрали спереди
    PneumoAngleCock *anglecock_bp_fwd = nullptr;
    /// Концевой кран тормозной магистрали сзади
    PneumoAngleCock *anglecock_bp_bwd = nullptr;
    QString     anglecock_bp_config = "pneumo-anglecock-BP";

    /// Рукав тормозной магистрали спереди
    PneumoHoseEPB   *hose_bp_fwd = nullptr;
    /// Рукав тормозной магистрали сзади
    PneumoHoseEPB   *hose_bp_bwd = nullptr;
    QString     hose_bp_module = "hose369a";
    QString     hose_bp_config = "pneumo-hose-BP369a-passcar";

    /// Тормозная рычажная передача
    BrakeMech   *brake_mech = nullptr;
    QString     brake_mech_config = "carbrakes-mech-composite";

    /// Передаточное число редуктора в подвагонном электрогенераторе
    double ip = 2.96;

    /// Регистратор параметров
    Registrator *registrator = nullptr;
    /// Признак включения регистрации
    bool is_Registrator_on = false;

    /// Тормозные башмаки
    std::vector<BrakeShoes *> brake_shoes;

    /// Триггер, управляющий установкой башмаков
    TriggerControl brake_shoes_set;

    /// Суммарное усилие от башмаков
    double shoesForce = 0.0;

    /// Чтение конфигурационного файла
    void loadConfig(QString cfg_path) override;


    /// Инициализация
    void initialization() override;

    /// Инициализация сцепных устройств
    void initCouplings(const QString& modules_dir, const QString& custom_cfg_dir);

    /// Инициализация тормозного оборудования
    void initBrakesEquipment(const QString& modules_dir, const QString& custom_cfg_dir);

    /// Инициализация ЭПТ
    void initEPB(const QString& modules_dir, const QString& custom_cfg_dir);

    /// Инициализация регистратора параметров в лог-файл
    void initRegistrator(const QString& modules_dir, const QString& custom_cfg_dir);

    /// Инициализация управления
    void initControl(const QString& modules_dir, const QString& custom_cfg_dir);


    /// Процесс симуляции
    void process(const simulator_time_t& t, const double& dt) override;

    /// Отладочная строка
    void debugPrint(const simulator_time_t& t, const double& dt);

    /// Сигналы для анимации
    void signalsOutput(const simulator_time_t& t, const double& dt);

    /// Сигналы для озвучки
    void soundsOutput(const simulator_time_t& t, const double& dt);


    /// Предварительные расчёты перед симуляцией
    void preStep(const double& t) override;

    /// Предварительный расчёт координат сцепных устройств
    void preStepCouplings(const double& t);


    /// Шаг моделирования
    void step(const double& t, const double& dt) override;

    /// Моделирование сцепных устройств
    void stepCouplings(const double& t, const double& dt);

    /// Моделирование тормозного оборудования
    void stepBrakesEquipment(const double& t, const double& dt);

    /// Моделирование ЭПТ
    void stepEPB(const double& t, const double& dt);

    /// Вывод параметров в лог-файл
    void stepRegistrator(const double& t, const double& dt);
};

#endif // PASSCAR_H
