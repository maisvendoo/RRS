#ifndef     ALSN_DECODER_H
#define     ALSN_DECODER_H

#include    <device.h>
#include    <timer.h>
#include    <ALSN-struct.h>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class DEVICE_EXPORT DecoderALSN : public Device
{
public:

    DecoderALSN(QObject* parent = nullptr);

    ~DecoderALSN() = default;

    void step(double t, double dt) override;

    ALSN getCode() const noexcept
    {
        return current_code;
    }

    void setCoilSignal(ALSN code) noexcept;

protected:

    /// Текущий принятый и расшифрованый код АЛСН
    ALSN current_code = ALSN::NO_CODE;

    ALSN last_code = ALSN::NO_CODE;

    static constexpr int NUM_VALUES = 3;

    /// Принятый от катушек код АЛСН
    std::array<ALSN, NUM_VALUES> recv_codes = { ALSN::NO_CODE };

    /// Таймер для обновления текущего кода
    Timer* update_timer = nullptr;

    void preStep(state_vector_t &Y, double t) override;

    void ode_system(const state_vector_t &Y,
                    state_vector_t &dYdt,
                    double t) override;

    void load_config(CfgReader &cfg) override;

protected slots:

    void slotOnUpdateTimer();
};

#endif
