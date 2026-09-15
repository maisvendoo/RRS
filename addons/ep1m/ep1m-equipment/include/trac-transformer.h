#ifndef     TRAC_TRANSFORMER_H
#define     TRAC_TRANSFORMER_H

#include    "device.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class TractionTransformer : public Device
{
public:

    TractionTransformer(QObject *parent = Q_NULLPTR);

    ~TractionTransformer() override;

    void setInputVoltage(double U1) { this->U1 = U1; }

    double getControlPowerVoltage() const { return U2cc; }

    double getFieldRectVoltage() const { return U2f; }

    double getUt1() const { return Ut1; }

    double getUt2() const { return Ut2; }

    double getUt3() const { return Ut3; }

    float getSoundSignal()
    {
        return sound_state.createSoundSignal();
    }

private:

    /// Напряжение на первичной обмотке
    double U1;

    /// Коэффициент передачи на обмотку питания цепей управления
    double Kcc;

    /// Напряжение обмотки питания собственных нужд
    double U2cc;

    /// Напряжениее на омотке питания ВУВ-118
    double U2f;

    /// Коэффициент передачи на четверть тяговой полуобмотки
    double K1_4;

    /// Коэфициент передачи на половину тяговой полуобмотки
    double K2_4;

    /// Коэффициент передачи на омотку питания ВУВ-118
    double Kf;

    /// Напряжение первой секции тяговогой обмотки (четверть)
    double Ut1;

    /// Напряжение второй секции тяговой обмотки (втрорая четверть)
    double Ut2;

    /// Напряжение третьей секции тяговой обмотки (последняя половина)
    double Ut3;

    sound_state_t sound_state;

    void ode_system(const state_vector_t &Y,
                    state_vector_t &dYdt,
                    double t) override;

    void stepDiscrete(double t, double dt) override;
};


#endif // TRAC_TRANSFORMER_H
