#ifndef     TEP70_MOTOR_COMPRESSOR_H
#define     TEP70_MOTOR_COMPRESSOR_H

#include    "motor-compressor-dc.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class TEP70MotorCompressor : public DCMotorCompressor
{
public:

    TEP70MotorCompressor(QObject *parent = Q_NULLPTR);

    ~TEP70MotorCompressor();

    virtual void step(double t, double dt);

    void setKontaktorState(size_t i, bool state);

private:

    /// Сопротивление двигателя
    double                          R0;

    /// Добавочное сопротивление в цепи якоря
    double                          rd;

    /// Ступени пускового реостата
    std::vector<double>             Rd;

    /// Контакторы пускового реостата
    std::vector<bool>               kontaktor_step;

    void load_config(CfgReader &cfg);
};

#endif // TEP70_MOTOR_COMPRESSOR_H
