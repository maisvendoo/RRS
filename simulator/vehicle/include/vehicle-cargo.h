//------------------------------------------------------------------------------
//
//      Cargo system (погрузка/разгрузка грузовых вагонов)
//      ТЗ "Реалистичная система погрузки и пассажиропотока"
//
//      Погрузка физически меняет массу вагона (через payload ПЕ):
//      ускорение/торможение/сцепки/расход энергии реагируют сразу.
//      Состояния: Empty -> Loading -> PartiallyLoaded -> Loaded ->
//      Unloading. Прерванная погрузка продолжается с того же места.
//      Центр масс груза смещает ЦМ вагона (влияет на осевые нагрузки
//      и поперечную устойчивость). Лимиты: масса/объём/осевая/груз.
//
//------------------------------------------------------------------------------

#ifndef     VEHICLE_CARGO_H
#define     VEHICLE_CARGO_H

#include    <QString>
#include    <QStringList>

#include    <cstddef>

#ifndef VEHICLE_EXPORT
    #if defined(VEHICLE_LIB)
        #define VEHICLE_EXPORT Q_DECL_EXPORT
    #else
        #define VEHICLE_EXPORT Q_DECL_IMPORT
    #endif
#endif

//------------------------------------------------------------------------------
/// Груз вагона
//------------------------------------------------------------------------------
class VEHICLE_EXPORT CargoSystem
{
public:

    /// Состояние (ТЗ, п.17)
    enum class State
    {
        Empty = 0,
        Loading = 1,
        PartiallyLoaded = 2,
        Loaded = 3,
        Unloading = 4
    };

    CargoSystem() = default;

    /// Загрузка секции [CargoWagon]
    void loadConfig(QString cfg_path);

    /// Секция [CargoWagon] присутствует в конфиге ПЕ
    /// (вагон управляет загрузкой динамически)
    bool isConfigured() const;

    /// Начать погрузку (вагон установлен в зоне, груз совместим)
    bool startLoading(const QString& cargo_type);

    /// Начать разгрузку
    bool startUnloading();

    /// Прервать операцию (состояние груза сохраняется, п.15)
    void stop();

    /// Шаг: rate - скорость оборудования точки, т/ч
    void step(double dt, double rate_t_per_hour);

    State getState() const;
    QString getCargoType() const;

    /// Масса груза, кг
    double getCargoMass() const;

    /// Доля заполнения 0..1
    double getFillLevel() const;

    /// Продольное смещение ЦМ груза от середины, м (+ к переду)
    double getLongitudinalShift() const;

    /// Поперечное смещение ЦМ груза, м (+ вправо)
    double getLateralShift() const;

    QString getLastError() const;

    /// Полная выгрузка (сброс)
    void reset();

    /// Засчитать доставку заказа (ТЗ "Погрузка", п.18 - экономика):
    /// вызывается моделью при завершённой разгрузке в точке с заказом
    void markDelivered(const QString& delivered_cargo, double tonnes);

    /// Доставлено грузов, шт (накопительный итог сценария)
    unsigned getDeliveredCargoCount() const;

    /// Доставлено груза, т (накопительный итог сценария)
    double getDeliveredTonnes() const;

private:

    State state = State::Empty;
    QString cargo_type = "";

    double max_load_kg = 68000.0;
    double cargo_mass = 0.0;

    /// Совместимые грузы (пусто - любые)
    QStringList allowed_cargo;

    /// Смещение ЦМ груза (неравномерное распределение, п.4)
    double com_longitudinal = 0.0;
    double com_lateral = 0.0;

    QString last_error = "";

    /// Секция [CargoWagon] найдена в конфиге
    bool configured = false;

    /// Накопительный итог доставки заказов (ТЗ п.18)
    unsigned delivered_cargo_count = 0;
    double delivered_tonnes = 0.0;
};

#endif // VEHICLE_CARGO_H
