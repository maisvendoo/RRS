#ifndef     TRAJECTORY_SPEEDMAP_H
#define     TRAJECTORY_SPEEDMAP_H

#include    "topology-trajectory-device.h"

//------------------------------------------------------------------------------
// Модуль путевой инфраструктуры с картой скоростей на траектории
//------------------------------------------------------------------------------
class TrajectorySpeedMap : public TrajectoryDevice
{
public:

    TrajectorySpeedMap(QObject *parent = Q_NULLPTR);

    ~TrajectorySpeedMap();

    /// Шаг симуляции
    void step(double t, double dt);

    std::vector<double> *getLimits();
    std::vector<double> *getLimitBegins();
    std::vector<double> *getLimitEnds();
    double getTrajLength();

protected:

    /// Ограничения скорости на траектории
    std::vector<double> limits = {};

    /// Траекторные координаты интервалов ограничения скорости
    std::vector<double> limit_begins = {};
    std::vector<double> limit_ends = {};

    /// Инициализация и чтение конфигурационного файла
    void load_config(CfgReader &cfg);
};

#endif // TRAJECTORY_SPEEDMAP_H
