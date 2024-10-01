#ifndef     ALSN_DECODER_H
#define     ALSN_DECODER_H

#include    <device.h>
#include    <timer.h>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class DEVICE_EXPORT DecoderALSN : public Device
{
public:

    DecoderALSN(QObject *parent = Q_NULLPTR);

    ~DecoderALSN();

    void step(double t, double dt) override;

protected:

    void preStep(state_vector_t &Y, double t) override;

    void ode_system(const state_vector_t &Y,
                    state_vector_t &dYdt,
                    double t) override;

    void load_config(CfgReader &cfg) override;
};

#endif
