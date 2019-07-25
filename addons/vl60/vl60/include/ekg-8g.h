#ifndef     EKG_8G_H
#define     EKG_8G_H

#include    "device.h"
#include    "timer.h"
#include    "km-state.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class EKG_8G : public Device
{
public:

    EKG_8G(QObject *parent = Q_NULLPTR);

    ~EKG_8G();

    void setKMstate(const km_state_t &km_state);

    float getSelsinPosition() const;

private:

    enum
    {
        NUM_POSITIONS = 38,
        PP_MIN = 17,
        PP_MAX = 22
    };

    /// Номер текущей позиции (с учетом переходных позиций)
    int  position;

    /// Номер заданной позиции
    int ref_position;

    /// Время переключения одной позиции, с
    double  switch_time;

    /// Таймер управления переключением позиций
    Timer   pos_switcher;

    /// Состояние контроллера машиниста
    km_state_t  km_state;

    /// Тригер для фиксации пуска
    Trigger      fix_start;

    /// Тригер для фиксации выключения
    Trigger      fix_off;

    void preStep(state_vector_t &Y, double t);

    void ode_system(const state_vector_t &Y, state_vector_t &dYdt, double t);

    void load_config(CfgReader &cfg);

    void stepDiscrete(double t, double dt);

private slots:

    void slotPosSwitch();
};

#endif // EKG_8G_H
