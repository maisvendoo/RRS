//------------------------------------------------------------------------------
//
//      Depot power system (деповское питание 380 В и заряд АБ)
//      ТЗ "Система деповского питания локомотива"
//
//      Внешнее питание - физически подключаемая система: источник 380 В
//      депо, кабель ограниченной длины, розетка локомотива. Блокировки:
//      нельзя подключить под напряжением, нельзя выдернуть кабель под
//      нагрузкой, движение с подключённым кабелем запрещено.
//      Цепь: 380 В -> вводной аппарат -> зарядное устройство ->
//      низковольтная сеть + аккумуляторная батарея (SoC/напряжение/ток/
//      температура). Без внешнего питания и незаведённом дизеле
//      вспомогательные потребители сажают АБ.
//
//------------------------------------------------------------------------------

#ifndef     VEHICLE_DEPOT_POWER_H
#define     VEHICLE_DEPOT_POWER_H

#include    <QString>

#include    <cstddef>

#ifndef VEHICLE_EXPORT
    #if defined(VEHICLE_LIB)
        #define VEHICLE_EXPORT Q_DECL_EXPORT
    #else
        #define VEHICLE_EXPORT Q_DECL_IMPORT
    #endif
#endif

//------------------------------------------------------------------------------
/// Деповское питание и аккумуляторная батарея ПЕ
//------------------------------------------------------------------------------
class VEHICLE_EXPORT DepotPowerSystem
{
public:

    DepotPowerSystem() = default;

    /// Загрузка секций [DepotPower] и [Battery]
    void loadConfig(QString cfg_path);

    //--------- Подключение кабеля (последовательность игрока) ---------

    /// Подключить кабель к розетке ПЕ (источник должен быть выключен).
    /// distance - расстояние от источника до ПЕ, м
    bool connectCable(double distance);

    /// Отключить кабель (источник должен быть выключен)
    bool disconnectCable();

    /// Включить/выключить питание источника (после подключения)
    bool setSourcePower(bool on);

    /// Включить/выключить вводной аппарат локомотива (после появления
    /// напряжения - шаг 8 последовательности игрока, ТЗ п.4)
    void setInputBreaker(bool on);

    /// Вводной аппарат включён
    bool isInputBreakerOn() const;

    /// Источник депо в зоне досягаемости (зона service.conf с ресурсом
    /// power; модель отмечает стоящие внутри ПЕ). cable_length - длина
    /// кабеля источника из конфига зоны, м
    void setSourceNearby(bool nearby, double cable_length = 25.0);

    /// Источник депо рядом с ПЕ (кабель дотянется)
    bool isSourceNearby() const;

    /// Длина кабеля ближайшего источника, м
    double getNearbyCableLength() const;

    /// Кабель подключён
    bool isCableConnected() const;

    /// Внешнее питание подано
    bool isExternalPower() const;

    /// Движение запрещено (кабель ещё подключён - потребитель
    /// обрежет тягу, ТЗ п.14)
    bool isMovementBlocked() const;

    /// Разъём повреждён (обрыв кабеля при движении - нужен ремонт)
    bool isConnectorDamaged() const;

    /// Подключённый источник: длина кабеля, м
    double getCableLength() const;

    //--------- Состояние батареи ---------

    /// Заряд АБ, 0..1
    double getBatteryCharge() const;

    /// Напряжение АБ, В
    double getBatteryVoltage() const;

    /// Ток заряда (положительный) / разряда (отрицательный), А
    double getBatteryCurrent() const;

    /// Температура АБ, град. C
    double getBatteryTemperature() const;

    /// Батарея критически разряжена (системы отключаются)
    bool isBatteryDead() const;

    //--------- Шаг ---------

    /// dt; aux_load_w - мощность низковольтных потребителей, Вт;
    /// engine_running - свой источник работает (заряд от генератора);
    /// vehicle_moving - ПЕ движется (движение с кабелем запрещено, п.14)
    void step(double dt, double aux_load_w, bool engine_running,
              bool vehicle_moving);

    /// Последняя ошибка блокировки (для подсказки игроку)
    QString getLastError() const;

    QString getDebugMsg() const;

private:

    // Источник депо (единственный ближайший; несколько - конфигом депо)
    double source_voltage = 380.0;
    double source_max_power = 30000.0;   ///< Вт
    double cable_length = 25.0;          ///< м
    bool source_on = false;

    /// Источник в зоне досягаемости (маршрутная колонка депо)
    bool source_nearby = false;
    double nearby_cable_length = 25.0;   ///< м

    bool cable_connected = false;
    bool external_power = false;
    bool move_violation = false;

    /// Разъём локомотива повреждён обрывом кабеля при движении
    bool connector_damaged = false;

    /// Зафиксировано ограничение тока пределом источника (журнал)
    bool src_limit_reported = false;

    // Вводной аппарат локомотива
    bool input_breaker_on = false;

    // Аккумуляторная батарея
    double battery_capacity = 550.0;     ///< А*ч
    double battery_charge = 0.9;         ///< 0..1
    double battery_min_voltage = 45.0;
    double battery_max_voltage = 58.0;
    double max_charge_current = 50.0;    ///< А
    double battery_temperature = 20.0;
    double battery_current = 0.0;

    // Зарядное устройство
    double charger_power = 5000.0;       ///< Вт

    QString last_error = "";
};

#endif // VEHICLE_DEPOT_POWER_H
