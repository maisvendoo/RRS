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
    void step(double t, double dt);

    void setNextSignalInfo(std::int8_t dir, ALSN code, double distance = 0.0, QString liter = "");

protected:

    /// Несущая частота сигнала, Гц
    double frequency = 0.0;

    /// Код от сигнала спереди
    ALSN code_from_fwd = ALSN::NO_CODE;
    /// Код от сигнала сзади
    ALSN code_from_bwd = ALSN::NO_CODE;

    /// Дистанция до сигнала спереди на следующих траекториях
    double distance_fwd = 0.0;
    /// Дистанция до сигнала сзади на следующих траекториях
    double distance_bwd = 0.0;

    /// Литер сигнала спереди
    QString next_liter_fwd = "";
    /// Литер сигнала сзади
    QString next_liter_bwd = "";

    /// Инициализация и чтение конфигурационного файла
    void load_config(CfgReader &cfg);

private:

    /// Очистка
    void clear_code();
};

#endif // TRAJECTORY_ALSN_H
