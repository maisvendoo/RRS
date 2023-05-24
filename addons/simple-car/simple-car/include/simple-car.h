#ifndef SIMPLE_CAR_H
#define SIMPLE_CAR_H

#include    "vehicle-api.h"
#include    "simple-car-signals.h"

#include    "registrator.h"

//---------------------------------------------------------------------
//
//---------------------------------------------------------------------
class SimpleCar : public Vehicle
{
public:
    /// Конструктор класса
    SimpleCar(QObject *parent = Q_NULLPTR);

    /// Деструктор класса
    ~SimpleCar();

    /// Инициализация тормозных приборов
    void initBrakeDevices(double p0, double pTM, double pFL);

private:

    // Старая пневносхема
    /// Главный резервуар
    Reservoir   *old_main_reservoir;

    /// Запасный резервуар
    Reservoir   *old_supply_reservoir;

    /// Тормозной цилиндр
    Reservoir   *old_brake_cylinder;

    /// Воздухораспределитель
    AirDistributor  *old_air_dist;

    // Новая пневносхема
    /// Главный резервуар
    Reservoir   *main_reservoir;

    /// Запасный резервуар
    Reservoir   *supply_reservoir;

    /// Тормозной цилиндр
    Reservoir   *brake_cylinder;

    /// Воздухораспределитель
    AirDistributor  *air_dist;

    /// Тормозная магистраль
    Reservoir   *brakepipe;

    /// Рукав тормозной магистрали спереди
    PneumoHose  *hose_tm_fwd;

    /// Рукав тормозной магистрали сзади
    PneumoHose  *hose_tm_bwd;

    /// Концевой кран тормозной магистрали спереди
    PneumoAngleCock  *anglecock_tm_fwd;

    /// Концевой кран тормозной магистрали сзади
    PneumoAngleCock  *anglecock_tm_bwd;

    /// Registrator
    Registrator *reg;

    /// Общая инициализация локомотива
    void initialization();

    /// Инициализация старой пневмосхемы
    void initPneumatics_old();

    /// Инициализация новой пневмосхемы
    void initPneumatics();

    /// Загрузка пользовательских параметров из конфига
    void loadConfig(QString cfg_path);

    /// Шаг моделирования систем единицы ПС
    void step(double t, double dt);

    /// Моделирование старой пневмосхемы
    void stepPneumatics_old(double t, double dt);

    /// Моделирование новой пневмосхемы
    void stepPneumatics(double t, double dt);

    /// Сигналы для анимации
    void stepSignalsOutput();

    /// Отладочная строка
    void stepDebugMsg(double t, double dt);

    /// Запись в лог-файл
    void stepRegistrator(double t, double dt);
};
#endif // SIMPLE_CAR_H
