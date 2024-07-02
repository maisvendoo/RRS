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

protected:

    /// Состояние свистка
    bool is_svistok;

    /// Состояние тифона
    bool is_tifon;

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
