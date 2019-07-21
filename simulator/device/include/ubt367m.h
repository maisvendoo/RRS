#ifndef     UBT_367M_H
#define     UBT_367M_H

#include    "brake-device.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class DEVICE_EXPORT BrakeLock : public BrakeDevice
{
public:

    BrakeLock(QObject *parent = Q_NULLPTR);

    ~BrakeLock();

    void init(double pTM, double pFL);

    void setCraneTMpressure(double press);

    void setLocoFLpressure(double press);

    double getState() const;

    double getCraneFLpressure() const;

    double getLocoTMpressure() const;

    float getCombCranePos() const;

    float getMainHandlePos() const;

private:

    double  V0;

    double  p1;

    int  state;

    double  crane_pTM;

    double  loco_pFL;

    double  crane_pFL;

    int    comb_crane_pos;

    bool   handle_unlocked;

    double  is_emerg_brake;

    double  is_comb_opened;

    enum
    {
        NUM_COEFFS = 4
    };

    std::array<double, NUM_COEFFS> K;

    Timer   *incCompCrane;
    Timer   *decCompCrane;

    void preStep(state_vector_t &Y, double t);

    void ode_system(const state_vector_t &Y, state_vector_t &dYdt, double t);

    void load_config(CfgReader &cfg);

    void stepKeysControl(double t, double dt);

    void combCraneInc();

    void combCraneDec();

    void setState(int state);
};

#endif // UBT367M_H
