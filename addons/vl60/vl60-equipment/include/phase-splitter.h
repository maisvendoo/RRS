#ifndef     PHASE_SPLITTER_H
#define     PHASE_SPLITTER_H

#include    "device.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class PhaseSplitter : public Device
{
public:

    PhaseSplitter(QObject *parent = Q_NULLPTR);

    ~PhaseSplitter();

    void setU_power(double value);

    float isNotReady() const;

    double getU_out() const;

    /// Состояние звука работы
    sound_state_t getSoundState(size_t idx = 0) const;

    /// Сигнал состояния звука работы
    float getSoundSignal(size_t idx = 0) const;

private:

    /// Максимальный крутящий момент
    double  Mmax;

    /// Критическое скольжение
    double  s_kr;

    /// Номинальное напряжение питания
    double  Un;

    /// Синхронная скорость
    double  omega0;

    /// Момент сопротивления на холостом ходу
    double  Mxx;

    /// Момент инерции системы "ротор + якорь ГУ"
    double  J;

    /// Текущее напряжение питания
    double  U_power;

    /// Уставка срабатывания реле оборотов
    double  omega_r;

    /// Признак запуска ФР (задает состояние контрольной лампы ФР на пульте)
    float   is_not_ready;

    double  k_eds;

    double  U_out;

    /// Состояние звука фазорасщепителя
    sound_state_t sound_state;

    void preStep(state_vector_t &Y, double t);

    void ode_system(const state_vector_t &Y, state_vector_t &dYdt, double t);

    void load_config(CfgReader &cfg);
};

#endif // PHASE_SPLITTER_H
