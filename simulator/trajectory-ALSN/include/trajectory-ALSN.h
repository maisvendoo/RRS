#ifndef     TRAJECTORY_ALSN_H
#define     TRAJECTORY_ALSN_H

#include    "topology-trajectory-device.h"
#include    "ALSN-struct.h"

#if defined(TRAJECTORY_ALSN_LIB)
    #define TRAJECTORY_ALSN_EXPORT   Q_DECL_EXPORT
#else
    #define TRAJECTORY_ALSN_EXPORT   Q_DECL_IMPORT
#endif

//------------------------------------------------------------------------------
// Модуль путевой инфраструктуры с рельсовыми цепями АЛСН
//------------------------------------------------------------------------------
class TRAJECTORY_ALSN_EXPORT TrajectoryALSN : public TrajectoryDevice
{
public:

    TrajectoryALSN(QObject *parent = Q_NULLPTR);

    ~TrajectoryALSN();

    /// Шаг симуляции
    void step(double t, double dt);

    void setSignalInfoFwd(ALSN code, double distance = 0.0, QString liter = "");
    void setSignalInfoBwd(ALSN code, double distance = 0.0, QString liter = "");

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
