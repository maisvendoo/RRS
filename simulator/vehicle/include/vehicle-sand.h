//------------------------------------------------------------------------------
//
//      Sand system (пескоподача локомотива)
//      ТЗ "Реалистичная система подачи песка локомотива"
//
//      Песок реально расходуется из бункера, его состояние (влажность,
//      замерзание) влияет на сыпучесть, форсунки засоряются, эффект
//      ограничен и зависит от состояния рельса. Подача подключается к
//      системе сцепления (WheelRailAdhesion::applySand) на оси под
//      работающими форсунками. Автоподача включает песок при
//      пробуксовке/юзе.
//
//------------------------------------------------------------------------------

#ifndef     VEHICLE_SAND_H
#define     VEHICLE_SAND_H

#include    <QString>

#include    <cstddef>
#include <vector>

class WheelRailAdhesion;

//------------------------------------------------------------------------------
/// Система пескоподачи локомотива
//------------------------------------------------------------------------------
class SandSystem
{
public:

    /// Состояние песка в бункере
    enum class SandState
    {
        Dry = 0,
        Damp = 1,
        Wet = 2,
        Frozen = 3
    };

    SandSystem() = default;

    /// Загрузка секции [Sand]
    void loadConfig(QString cfg_path, std::size_t num_axis);

    /// Ручное управление подачей (вперёд/назад выбирается по знаку
    /// скорости; форсунки передних/задних осей)
    void setSanding(bool enabled);

    /// Режим автоматической подачи (пробуксовка/юз)
    void setAutoSanding(bool enabled);

    /// Шаг системы.
    /// slip - есть пробуксовка/юз хотя бы одной оси;
    /// adhesion - для применения эффекта; rail_wet/rail_ice/rail_contam -
    /// состояние рельса (эффективность песка от состояния);
    /// humidity 0..1, temperature - погода
    void step(double dt,
              double velocity,
              bool slip,
              WheelRailAdhesion& adhesion,
              double rail_wet,
              double rail_ice,
              double rail_contam,
              double humidity,
              double temperature);

    /// Идёт фактическая подача песка
    bool isFeeding() const;

    /// Система активна (секция [Sand] в конфиге ПЕ)
    bool isEnabled() const;

    /// Запас песка, кг
    double getAmount() const;

    /// Ёмкость бункера, кг
    double getCapacity() const;

    /// Влажность песка 0..1
    double getMoisture() const;

    /// Состояние песка
    SandState getSandState() const;

    /// Засорение форсунок 0..1 (усреднённое)
    double getNozzleClogging() const;

    /// Пополнение песка (заправка)
    void refill(double amount);

    /// Обслуживание: чистка форсунок
    void service();

    QString getDebugMsg() const;

private:

    /// Песочница есть у этой ПЕ (секция [Sand] в конфиге).
    /// По умолчанию выключена: вагоны песок не подают
    bool enabled = false;

    /// Ручное включение
    bool manual_on = false;

    /// Автоматический режим
    bool auto_mode = true;

    /// Состояние автоподачи (гистерезис)
    bool auto_active = false;
    double auto_delay = 0.0;

    /// Бункер
    double capacity = 0.0;         ///< кг
    double amount = 0.0;           ///< кг
    double moisture = 0.05;        ///< 0..1

    /// Базовый расход, кг/с (на все форсунки)
    double base_rate = 2.5;

    /// Сыпучесть падает с влажностью (порог полной потери)
    double wet_flow_limit = 0.35;

    /// Засорение форсунок 0..1
    std::vector<double> nozzle_clog;

    /// Ось - передняя (первые num_front_axles) или задняя группа
    std::size_t num_axis = 4;

    /// Порог пробуксовки для автоподачи
    double slip_threshold = 0.15;

    /// Задержки включения/выключения автоподачи, с
    double activation_delay = 0.3;
    double release_delay = 2.0;

    /// Текущий буст сцепления от песка
    double sand_boost = 0.15;

    /// Фактическая подача в данный момент
    bool feeding = false;

    /// Последняя температура воздуха (для состояния песка)
    double last_temperature = 15.0;
};

#endif // VEHICLE_SAND_H
