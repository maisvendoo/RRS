#ifndef     BS_002_H
#define     BS_002_H

#include    "device.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
enum
{
    SM_LAMPS_NUMBER = 34
};

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
enum
{
    // Верхний ряд ламп
    SM_LAMP_SI = 0,
    SM_LAMP_VIP = 1,
    SM_LAMP_TC3 = 2,
    SM_LAMP_TC2 = 3,
    SM_LAMP_TC1 = 4,
    SM_LAMP_TRANS = 5,
    SM_NN_PCHF = 6,
    SM_RKZ = 7,
    SM_RN = 8,
    SM_DM2 = 9,
    SM_DM1 = 10,
    SM_ZB = 11,
    SM_PS = 12,
    SM_REZERV1 = 13,
    SM_REZERV2 = 14,
    SM_RZ = 15,
    SM_GV = 16,

    // Нижний ряд ламп
    SM_VUV = 17,
    SM_LOW_FREQ = 18,
    SM_WEEK_FIELD = 19,
    SM_REZERV3 = 20,
    SM_DB = 21,
    SM_V4 = 22,
    SM_V3 = 23,
    SM_V2 = 24,
    SM_V1 = 25,
    SM_MK2 = 26,
    SM_MK1 = 27,
    SM_TD6 = 28,
    SM_TD5 = 29,
    SM_TD4 = 30,
    SM_TD3 = 31,
    SM_TD2 = 32,
    SM_TD1 = 33,
};

//------------------------------------------------------------------------------
//  Блок сигнализации БС-002
//------------------------------------------------------------------------------
class SignalizationModule : public Device
{
public:

    SignalizationModule(QObject *parent = Q_NULLPTR);

    ~SignalizationModule();

    void setVoltage(double Upow) { this->Upow = Upow; }

    void setLampInputSignal(size_t idx, bool lamp_signal)
    {
        lamp_input[idx] = lamp_signal;
    }

    bool getLampState(size_t idx) const
    {
        return lamp_state[idx];
    }

private:

    double Upow;

    std::array<bool, SM_LAMPS_NUMBER> lamp_input;

    std::array<bool, SM_LAMPS_NUMBER> lamp_state;

    void preStep(state_vector_t &Y, double t);

    void ode_system(const state_vector_t &Y,
                            state_vector_t &dYdt,
                            double t);

    void load_config(CfgReader &cfg);
};

#endif // BS_002_H
