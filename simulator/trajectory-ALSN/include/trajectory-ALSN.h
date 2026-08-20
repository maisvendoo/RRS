#ifndef     TRAJECTORY_ALSN_H
#define     TRAJECTORY_ALSN_H

#include    "topology-trajectory-device.h"
#include    "ALSN-struct.h"

//------------------------------------------------------------------------------
// Модуль путевой инфраструктуры с рельсовыми цепями АЛСН
//------------------------------------------------------------------------------
class TrajectoryALSN : public TrajectoryDevice
{
public:

    TrajectoryALSN(QObject *parent = nullptr);

    ~TrajectoryALSN();

    /// Шаг симуляции
    void step(double t, double dt) override;

    QByteArray serialize() const override;
    void deserialize(QByteArray& data) override;
    void getDrawElements(std::vector<draw_line_t>& lines, std::vector<draw_circle_t>& circles, const double scale) override;

    void setNextSignalInfo(std::int8_t dir, ALSN code, double distance, const QString& liter);

private:

    /// Несущая частота сигнала, Гц
    double frequency = 0.0;

    /// Код от сигнала спереди
    ALSN code_from_fwd = ALSN::NO_CODE;
    ALSN prev_code_from_fwd = ALSN::NO_CODE;
    /// Код от сигнала сзади
    ALSN code_from_bwd = ALSN::NO_CODE;
    ALSN prev_code_from_bwd = ALSN::NO_CODE;

    /// Координаты занятого участка траектории
    /// (от начала первой ПЕ до конца последней ПЕ);
    /// между ними сигнала АЛСН нет,
    /// так как он зашунтирован колёсными парами
    double busy_begin_coord = -1.0;
    double prev_busy_begin_coord = -1.0;
    double busy_end_coord = -1.0;
    double prev_busy_end_coord = -1.0;

    /// Дистанция до сигнала спереди на следующих траекториях
    double distance_fwd = 0.0;
    /// Дистанция до сигнала сзади на следующих траекториях
    double distance_bwd = 0.0;

    /// Литер сигнала спереди
    QString next_liter_fwd = "";
    /// Литер сигнала сзади
    QString next_liter_bwd = "";

    /// Инициализация и чтение конфигурационного файла
    void load_config(CfgReader &cfg) override;

    /// Очистка
    void clear_code();
};

#endif // TRAJECTORY_ALSN_H
