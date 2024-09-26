#ifndef     ALSN_UKBM_H
#define     ALSN_UKBM_H

#include    <device.h>

enum
{
    RED_LAMP = 0,
    RED_YELLOW_LAMP = 1,
    YELLOW_LAMP = 2,
    GREEN_LAMP = 3,
    WHITE_LAMP = 4
};

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class SafetyDevice : public Device
{
public:

    SafetyDevice(QObject *parent = Q_NULLPTR);

    ~SafetyDevice();

    void step(double t, double dt) override;

    /// Прием кода АЛСН
    void setAlsnCode(int code_alsn)
    {
        old_code_alsn = this->code_alsn;
        this->code_alsn = code_alsn;
    };

    /// Прием состояния РБ
    void setRBstate(bool state) { state_RB = state; };

    /// Прием состояния РБС
    void setRBSstate(bool state) { state_RBS = state; };

    /// Прием скорости от скоростемера
    void setVelocity(double v) { v_kmh = v * Physics::kmh; }

    void setKeyEPK(bool key_epk) { this->key_epk = key_epk; }

    /// Выдача состояния цепи удерживающей катушки ЭПК
    bool getEPKstate() { return epk_state.getState(); };

    float getGreenLamp() const { return static_cast<float>(lamps[GREEN_LAMP]); }

    float getYellowLamp() const { return static_cast<float>(lamps[YELLOW_LAMP]); }

    float getRedYellowLamp() const { return static_cast<float>(lamps[RED_YELLOW_LAMP]); }

    float getRedLamp() const { return static_cast<float>(lamps[RED_LAMP]); }

    float getWhiteLamp() const { return static_cast<float>(lamps[WHITE_LAMP]); }

private:

    int code_alsn;

    int old_code_alsn;

    bool state_RB;

    bool state_RBS;

    bool state_EPK;

    double v_kmh;

    bool key_epk;

    Trigger is_red;

    std::array<bool, 5> lamps;

    Trigger epk_state;

    Timer *safety_timer;

    void preStep(state_vector_t &Y, double t) override;

    void ode_system(const state_vector_t &Y, state_vector_t &dYdt, double t) override;

    void load_config(CfgReader &cfg) override;

    void alsn_process(int code_alsn);

    void off_all_lamps();

    void lamp_on(size_t lamp_idx);

    bool is_lamp_on(size_t lamp_idx);

    void code_to_log();

private slots:

    void onSafetyTimer();
};

#endif // ALSN_UKBM_H
