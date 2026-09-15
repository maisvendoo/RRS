#ifndef     SHUNTS_MODULE_H
#define     SHUNTS_MODULE_H

#include    "device.h"
#include    "shunts-module-defines.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class ShuntsModule : public Device
{
public:

    ShuntsModule(QObject *parent = Q_NULLPTR);

    ~ShuntsModule();

    void setStep(size_t step)
    {
        cur_step = cut(step,
                       static_cast<size_t>(STEP0),
                       static_cast<size_t>(STEP3));
    }

    double getBeta() const
    {
        return beta[cur_step];
    }

private:

    /// Текущая ступень ослабления возбуждения от МСУД-Н
    size_t cur_step;

    /// Величина ступени ослабления возбуждения
    std::array<double, NUM_STEPS> beta;

    void preStep(state_vector_t &Y, double t);

    void ode_system(const state_vector_t &Y, state_vector_t &dYdt, double t);

    void load_config(CfgReader &cfg);
};

#endif // SHUNTS_MODULE_H
