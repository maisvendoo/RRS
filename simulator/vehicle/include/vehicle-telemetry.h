//------------------------------------------------------------------------------
//
//      Реестр источников телеметрии подвижного состава
//      (регистрация скорректированного api 2026-09-09)
//
//------------------------------------------------------------------------------
/*!
 * \file
 * \brief  Побочные данные приборов ПС для кассеты регистрации и сессий
 *
 * Класс Vehicle остаётся интерфейсом связи с физикой (как в апстриме),
 * поэтому адресация приборов конкретного ПС (давления тормозной
 * системы, ток ТЭД) живёт здесь: аддон ПС привязывает свои лямбды,
 * модель читает значения по указателю на ПЕ.
 */

#ifndef     VEHICLE_TELEMETRY_H
#define     VEHICLE_TELEMETRY_H

#include    <QtGlobal>
#include    <functional>
#include    <unordered_map>

#ifndef VEHICLE_EXPORT
    #if defined(VEHICLE_LIB)
        #define VEHICLE_EXPORT Q_DECL_EXPORT
    #else
        #define VEHICLE_EXPORT Q_DECL_IMPORT
    #endif
#endif

class Vehicle;

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class VEHICLE_EXPORT VehicleTelemetry
{
public:

    /// Источники аналоговых каналов ПС (единицы СИ: МПа, А);
    /// пустая лямбда - канала нет
    struct Sources
    {
        /// Давление в уравнительном резервуаре (кран машиниста)
        std::function<double()> equalizing_reservoir;
        /// Давление в тормозной магистрали
        std::function<double()> brake_pipe;
        /// Давление в тормозных цилиндрах
        std::function<double()> brake_cylinder;
        /// Давление в главном резервуаре
        std::function<double()> main_reservoir;
        /// Ток тяговых двигателей (средний/суммарный Ia)
        std::function<double()> traction_current;
    };

    /// Единственный экземпляр (живёт в vehicle.dll)
    static VehicleTelemetry& instance();

    /// Привязать источники ПС (вызывает аддон при инициализации)
    void bind(Vehicle* vehicle, Sources sources);

    /// Отвязать ПС (вызывает модель при удалении ПЕ)
    void unbind(Vehicle* vehicle);

    /// Значения каналов; без источника - -1.0
    double equalizingReservoirPressure(const Vehicle* vehicle) const;
    double brakePipePressure(const Vehicle* vehicle) const;
    double brakeCylinderPressure(const Vehicle* vehicle) const;
    double mainReservoirPressure(const Vehicle* vehicle) const;
    double tractionCurrent(const Vehicle* vehicle) const;

private:

    VehicleTelemetry() = default;

    const Sources* sourcesOf(const Vehicle* vehicle) const;

    std::unordered_map<const Vehicle*, Sources> registry;
};

#endif // VEHICLE_TELEMETRY_H
