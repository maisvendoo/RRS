//------------------------------------------------------------------------------
//
//      Passenger system (посадка/высадка пассажиров)
//      ТЗ "Реалистичная система погрузки и пассажиропотока", п.19-35
//
//      Пассажиры - масса (средняя с багажом ~90 кг): тяга/торможение/
//      расход реагируют через payload ПЕ. Посадка/высадка идут через
//      двери: скорость зависит от числа дверей и очереди; при полной
//      вместимости посадка прекращается (пассажиры уходят в другие
//      вагоны через распределение по составу).
//
//------------------------------------------------------------------------------

#ifndef     VEHICLE_PASSENGERS_H
#define     VEHICLE_PASSENGERS_H

#include    <QString>

#include    <cstddef>

//------------------------------------------------------------------------------
/// Пассажирский вагон
//------------------------------------------------------------------------------
class PassengerSystem
{
public:

    PassengerSystem() = default;

    /// Загрузка секции [PassengerCar]
    void loadConfig(QString cfg_path);

    /// Секция [PassengerCar] присутствует в конфиге ПЕ
    bool isConfigured() const;

    /// У вагона есть откидная/выдвижная лестница (ключ HasStairs,
    /// по умолчанию есть - проводник применяет её на низкой платформе)
    bool hasStairs() const;

    /// Двери открыты/закрыты (посадка только при открытых, п.28)
    void setDoorsOpen(bool open);
    bool isDoorsOpen() const;

    /// Идёт посадка/высадка (команда от системы станции)
    void beginBoarding(int waiting_on_platform);
    void beginAlighting(int exiting_count);

    /// Поток посадки/высадки ещё идёт (для проводников, п.16 ТЗ
    /// "Система проводников": дверь не закрывается до завершения)
    bool isBoardingInProgress() const;
    bool isAlightingInProgress() const;

    /// Шаг (скорость потоков из вместимости/дверей/очередей)
    void step(double dt);

    /// Число пассажиров в вагоне
    int getPassengerCount() const;

    /// Вместимость
    int getCapacity() const;

    /// Есть свободные места
    bool hasFreeSpace() const;

    /// Масса пассажиров, кг (90 кг средняя с багажом, п.32)
    double getPassengerMass() const;

    /// Скорость посадки с учётом очередей, пасс/с (для распределения)
    double getBoardingRate() const;

    /// Ждут посадки из этого вагона (неразмещённые)
    int getOverflowQueue() const;

private:

    int capacity = 100;
    int passengers = 0;

    int doors = 2;
    double door_width = 0.9;        ///< м

    /// Базовая скорость прохода пассажира через дверь, пасс/с на дверь
    double pass_rate = 0.7;

    bool doors_open = false;

    /// Поток посадки: ожидающие,_rate
    int boarding_queue = 0;

    /// Поток высадки: сколько должны выйти
    int alighting_left = 0;

    /// Переполнение: не поместились (передаются другим вагонам)
    int overflow = 0;

    /// Накопители дробных потоков (пасс/с < 1 не теряются)
    double boarding_accum = 0.0;
    double alighting_accum = 0.0;

    /// Секция [PassengerCar] найдена в конфиге
    bool configured = false;

    /// Есть откидная лестница (низкие платформы)
    bool has_stairs = true;
};

#endif // VEHICLE_PASSENGERS_H
