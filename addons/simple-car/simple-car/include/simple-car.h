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

    /// Номер Debug-строки
    size_t debug_num;

    /// Тормозная магистраль
    Reservoir   *brakepipe;

    /// Воздухораспределитель
    AirDistributor  *air_dist;

    /// Тормозной цилиндр
    Reservoir   *brake_cylinder;

    /// Запасный резервуар
    Reservoir   *supply_reservoir;

    /// Рукав тормозной магистрали спереди
    PneumoHose  *hose_bp_fwd;

    /// Рукав тормозной магистрали сзади
    PneumoHose  *hose_bp_bwd;

    /// Концевой кран тормозной магистрали спереди
    PneumoAngleCock  *anglecock_bp_fwd;

    /// Концевой кран тормозной магистрали сзади
    PneumoAngleCock  *anglecock_bp_bwd;

    /// Registrator
    Registrator *reg;

    /// Общая инициализация локомотива
    void initialization();

    /// Инициализация новой пневмосхемы
    void initPneumatics();

    /// Загрузка пользовательских параметров из конфига
    void loadConfig(QString cfg_path);

    /// Шаг моделирования систем единицы ПС
    void step(double t, double dt);

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
