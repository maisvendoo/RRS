//------------------------------------------------------------------------------
//
//      Tunnel aerodynamics (эффект "воздушного поршня" в тоннеле)
//      ТЗ "43-47", раздел 3
//
//      Поезд в тоннеле вытесняет воздух: чем меньше отношение сечений
//      тоннель/поезд, тем сильнее рост аэродинамического сопротивления.
//      Вход и выход плавные: коэффициент погружения 0..1 растёт, пока
//      первая ось заходит за портал, и падает, когда последняя ось
//      выходит (резкого переключения OUTSIDE -> TUNNEL нет).
//
//      Зоны тоннелей задаёт модель маршрута (setZones, пикетаж в системе
//      координат пути); система только хранит зоны, считает погружение
//      конкретной ПЕ и прибавку к сопротивлению (getResistanceForce),
//      которая складывается с основным сопротивлением в mainResist.
//
//------------------------------------------------------------------------------

#ifndef     VEHICLE_TUNNEL_H
#define     VEHICLE_TUNNEL_H

#include    <QString>

#include    <cstddef>
#include <utility>
#include <vector>

#ifndef VEHICLE_EXPORT
    #if defined(VEHICLE_LIB)
        #define VEHICLE_EXPORT Q_DECL_EXPORT
    #else
        #define VEHICLE_EXPORT Q_DECL_IMPORT
    #endif
#endif

//------------------------------------------------------------------------------
/// Аэродинамика тоннеля для единицы ПС
//------------------------------------------------------------------------------
class VEHICLE_EXPORT TunnelAerodynamics
{
public:

    /// Зона тоннеля: [начало, конец] в координате пути, м
    using Zone = std::pair<double, double>;

    TunnelAerodynamics() = default;

    /// Загрузка секции [TunnelAero]
    void loadConfig(QString cfg_path);

    /// Задать зоны тоннелей маршрута (пикетаж; загрузку выполняет модель)
    void setZones(const std::vector<std::pair<double, double>>& zones);

    /// Координата точки в тоннеле (хотя бы одна зона накрывает), м
    bool inTunnel(double coord) const;

    /// Обновить координату центра ПЕ на линии (та же система, что зоны)
    void setCoordinate(double railway_coord);

    /// Шаг: пересчёт погружения ПЕ в тоннель.
    /// dt - время; velocity - скорость (для кэша сопротивления не нужна,
    /// сила считается по запросу); train_length - длина ПЕ, м
    void step(double dt, double velocity, double train_length);

    /// Погруженность в тоннель 0..1 (доля длины ПЕ внутри зон)
    double getImmersion() const;

    /// Дополнительное сопротивление "поршня", Н (>= 0; 0 вне тоннеля):
    /// F = k * rho/2 * A_train * (A_train/A_tunnel)^2 * v^2 * immersion
    double getResistanceForce(double velocity) const;

    /// Есть настроенные зоны
    bool isConfigured() const;

    QString getDebugMsg() const;

private:

    bool enabled = true;

    /// Зоны тоннелей (отсортированы по началу)
    std::vector<Zone> zones;

    /// Координата центра ПЕ, м
    double coord = 0.0;

    /// Длина ПЕ последнего шага, м
    double train_length = 15.0;

    /// Погруженность 0..1
    double immersion = 0.0;

    // Параметры (конфиг [TunnelAero])

    /// Площадь сечения тоннеля, м^2 (однопутный ~35-50)
    double tunnel_area = 35.0;

    /// Площадь миделевого сечения ПЕ, м^2 (габарит ~3.3 x 3.2)
    double train_area = 10.6;

    /// Коэффициент поршневого давления (подгонка под тип тоннеля)
    double piston_coeff = 1.0;

    /// Плотность воздуха, кг/м^3
    double air_density = 1.225;
};

#endif // VEHICLE_TUNNEL_H
