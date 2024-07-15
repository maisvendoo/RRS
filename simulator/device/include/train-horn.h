#ifndef     TRAIN_HORN_H
#define     TRAIN_HORN_H

#include    "device.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class DEVICE_EXPORT TrainHorn : public Device
{
public:

    TrainHorn(QObject *parent = Q_NULLPTR);

    virtual ~TrainHorn();

    void step(double t, double dt);

    /// Задать состояние свистка
    void setSvistokOn(bool state);

    /// Состояние свистка
    bool isSvistok() const;

    /// Задать состояние тифона
    void setTifonOn(bool state);

    /// Состояние тифона
    bool isTifon() const;

    /// Задать давление от питательной магистрали, МПа
    void setFLpressure(double value);

    /// Поток в питательную магистраль
    double getFLflow() const;

    enum {
        NUM_SOUNDS = 2,
        SVISTOK_SOUND = 0,
        TIFON_SOUND = 1
    };
    /// Состояние звука
    sound_state_t getSoundState(size_t idx = SVISTOK_SOUND) const;

    /// Сигнал состояния звука
    float getSoundSignal(size_t idx = SVISTOK_SOUND) const;

protected:

    /// Состояние звуков
    std::array<sound_state_t, NUM_SOUNDS> sounds;

    /// Давление питательной магистрали, МПа
    double pFL;

    /// Поток в питательную магистраль
    double QFL;

    /// Номинальное давление для работы свистка и тифона, МПа
    double p_nom;

    /// Коэффициент потока - расхода воздуха на свисток
    double k_svistok;

    /// Коэффициент потока - расхода воздуха на тифон
    double k_tifon;

    void ode_system(const state_vector_t &Y, state_vector_t &dYdt, double t);

    void load_config(CfgReader &cfg);

    virtual void stepKeysControl(double t, double dt);
};

#endif // TRAIN_HORN_H
