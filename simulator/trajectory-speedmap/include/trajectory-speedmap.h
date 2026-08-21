#ifndef     TRAJECTORY_SPEEDMAP_H
#define     TRAJECTORY_SPEEDMAP_H

#include    "topology-trajectory-device.h"
#include    <speed-limit-source.h>

//------------------------------------------------------------------------------
// Модуль путевой инфраструктуры с картой скоростей на траектории
//------------------------------------------------------------------------------
class TrajectorySpeedMap : public TrajectoryDevice, public SpeedLimitSource
{
public:

    TrajectorySpeedMap(QObject *parent = nullptr);

    ~TrajectorySpeedMap();

    /// Шаг симуляции
    void step(double t, double dt) override;
/* TODO
    QByteArray serialize() const override;
    void deserialize(QByteArray& data) override;
    void getDrawElements(std::vector<draw_line_t>& lines, std::vector<draw_circle_t>& circles, const double scale) override;
*/
    std::vector<double> *getLimits();
    std::vector<double> *getLimitBegins();
    std::vector<double> *getLimitEnds();
    double getTrajLength();

    std::vector<speed_limit_interval_t> getSpeedLimits() const override;

protected:

    /// Ограничения скорости на траектории
    std::vector<double> limits = {};

    /// Траекторные координаты интервалов ограничения скорости
    std::vector<double> limit_begins = {};
    std::vector<double> limit_ends = {};

    /// Инициализация и чтение конфигурационного файла
    void load_config(CfgReader &cfg) override;
};

#endif // TRAJECTORY_SPEEDMAP_H
