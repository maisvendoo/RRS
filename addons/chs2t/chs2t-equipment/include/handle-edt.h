//------------------------------------------------------------------------------
//
//      Рукоятка задатчика тормозного усилия для реостатного тормоза
//      электровозов серии ЧС ("карандаш")
//
//------------------------------------------------------------------------------

#ifndef     HANDLE_EDT_H
#define     HANDLE_EDT_H

#include    "device.h"
#include    "timer.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class HandleEDT : public Device
{
public:

    HandleEDT(QObject *parent = Q_NULLPTR);

    ~HandleEDT();    

    void setBrakeKey(int key) { brakeKey = key; }

    void setReleaseKey(int key) { releaseKey = key; }

    float getHandlePos() const { return static_cast<float>(pos) / 2.0f; }

    double getControlSignal() const { return control_signal; }

private:

    int brakeKey;

    int releaseKey;

    int pos;

    int pos_ref;

    double control_signal;

    enum
    {
        POS_RELEASE = 0,
        POS_HOLD = 1,
        POS_BRAKE = 2
    };

    enum
    {
        NUM_COEFFS = 3
    };

    std::array<double, NUM_COEFFS> K;

    Timer motionTimer;

    void preStep(state_vector_t &Y, double t);

    void ode_system(const state_vector_t &Y, state_vector_t &dYdt, double t);

    void load_config(CfgReader &cfg);

    void stepKeysControl(double t, double dt);

    void stepExternalControl(double t, double dt);

private slots:

    void slotHandleMove();
};

#endif // HANDLEEDT_H
