//------------------------------------------------------------------------------
//
//      Магистральный пассажирский тепловоз ТЭП70.
//      Дополнение для Russian Railway Simulator (RRS)
//
//      (c) RRS development team:
//          Дмитрий Притыкин (maisvendoo),
//          Роман Бирюков (РомычРЖДУЗ)
//
//      Дата: 12/05/2019
//
//------------------------------------------------------------------------------
#ifndef     TEP70_H
#define     TEP70_H

#include    "vehicle-api.h"
#include    "tep70-headers.h"
#include    "tep70-signals.h"

#include    "battery.h"
#include    "relay.h"
#include    "fuel-tank.h"
#include    "electric-fuel-pump.h"
#include    "disel.h"
#include    "time-relay.h"
#include    "electric-oil-pump.h"
#include    "starter-generator.h"

/*!
 * \class
 * \brief Основной класс, описывающий весь тепловоз
 */
//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class TEP70 : public Vehicle
{
public:

    /// Конструктор
    TEP70();

    /// Деструктор
    ~TEP70();

private:

    /// Контроллер машиниста
    ControllerKM2202    *km;

    /// Аккумуляторная батарея
    Battery             *battery;

    /// Контактор топливного насоса (КТН)
    Relay               *kontaktor_fuel_pump;

    /// Топливный бак
    FuelTank            *fuel_tank;

    /// Электрический топливный насос (ЭНТ)
    ElectricFuelPump    *electro_fuel_pump;

    /// Дизель
    Disel               *disel;

    /// Реле РУ8
    Relay               *ru8;

    /// Контактор маслопрокачивающего насоса (КМН)
    Relay               *kontaktor_oil_pump;

    /// Реле времени прокачки масла
    TimeRelay           *oilpump_time_relay;

    /// Реле времени прокрутки стартера
    TimeRelay           *starter_time_relay;

    /// Электрический маслопрокачивающий насос (ЭМН)
    ElectricOilPump     *electro_oil_pump;

    /// Стратер-генератор
    StarterGenerator    *starter_generator;

    /// Контактор стартер-генератора (КД)
    Relay               *kontaktor_starter;

    /// Реле РУ10
    Relay               *ru10;

    /// Реле РУ6
    Relay               *ru6;

    /// Реле РУ42
    Relay               *ru42;

    /// Реле РУ7
    Relay               *ru7;

    /// Реле РУ15
    Relay               *ru15;

    /// Блок-магнит МВ6
    Relay               *mv6;

    /// Вентиль топливных насосов (ВТН)
    Relay               *vtn;

    /// Реле РУ4
    Relay               *ru4;

    /// Реле времени РВ4
    TimeRelay           *rv4;

    /// Кнопка "Пуск дизеля"
    bool    button_disel_start;

    /// Кнопка "Отпуск тормозов"
    bool    button_brake_release;

    /// Кнопка "Свисток"
    bool    button_svistok;

    /// Кнопка "Тифон"
    bool    button_tifon;

    /// Напряжение цепей управления
    double  Ucc;

    /// Ток цепей управления
    double  Icc;

    /// АЗВ "Управление общее"
    Trigger azv_common_control;

    /// АЗВ "Управление тепловозом"
    Trigger azv_upr_tepl;

    /// АЗВ "Топливный насос"
    Trigger azv_fuel_pump;

    /// АЗВ "ЭДТ"
    Trigger azv_edt_on;

    /// АЗВ "Тормоз питание"
    Trigger azv_edt_power;

    /// АЗВ "ЭПТ"
    Trigger azv_ept_on;

    /// Тумблер "Напряжение ЦУ. Напряжение ЭПТ"
    Trigger tumbler_voltage;

    /// Тумблер "Аварийная остановка дизеля"
    Trigger tumbler_disel_stop;

    /// Тумблер "Ослабление поля I ступени руч./авт."
    Switcher tumbler_field_weak1;

    /// Тумблер "Ослабление поля II ступени руч./авт."
    Switcher tumbler_field_weak2;

    /// Тумблер "Управление жалюзи воды руч./авт."
    Switcher tumbler_water_zaluzi;

    /// Тумблер "Управление жалюзи масла руч./авт."
    Switcher tumbler_oil_zaluzi;

    /// Инициализация всех систем тепловоза
    void initialization();

    /// Инициализация органов управления в кабине
    void initCabineControls();

    /// Инициализация цепей управления
    void initControlCircuit();

    /// Инициализация топливной системы
    void initFuelSystem();

    /// Инициализация дизеля
    void initDisel();

    /// Инициализация маслянной системы
    void initOilSystem();

    /// Инициализация звуков
    void initSounds();

    /// Шаг моделирования всех систем локомотива в целом
    void step(double t, double dt);

    /// Шаг моделирования органов управления в кабине
    void stepCabineControls(double t, double dt);

    /// Шаг моделирования цепей управления
    void stepControlCircuit(double t, double dt);

    /// Шаг моделирования топливной системы
    void stepFuelSystem(double t, double dt);

    /// Шаг моделирования дизеля
    void stepDisel(double t, double dt);

    /// Шаг моделирвания масляной системы
    void stepOilSystem(double t, double dt);

    /// Вывод сигналов для анимаций
    void stepSignalsOutput(double t, double dt);

    /// Вывод отладочной строки
    void debugOutput(double t, double dt);

    /// Формирование состояния сигнальных ламп
    float getLampState(double signal);

    /// Загрузка данных из конфигурационных файлов
    void loadConfig(QString cfg_path);

    /// Обработка клавиш
    void keyProcess();
};

#endif // TEP70_H
