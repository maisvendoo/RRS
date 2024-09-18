#ifndef     COMBINE_RELAY_H
#define     COMBINE_RELAY_H

#include    <relay.h>
#include    <polar-hysteresis.h>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class DEVICE_EXPORT CombineRelay : public Relay
{
public:

    CombineRelay(size_t num_neutral_contacts = 1,
                 size_t num_plus_contacts = 1,
                 size_t num_minus_contscts = 1,
                 QObject *parent = Q_NULLPTR);

    ~CombineRelay();

    void setInitPlusContactState(size_t idx, bool state);

    void setInitMinusContactState(size_t idx, bool state);

    bool getPlusContactState(size_t idx) const;

    bool getMinusContactState(size_t idx) const;

private:

    /// Контакты, активируемые при положительной полярности
    std::vector<bool> plus_contact;

    /// Контакты, активируемые при отрицательной полярности
    std::vector<bool> minus_contact;

    /// Двухсторонний гистерези с центральной симметрией
    PolarHysteresis *polar_hysteresis;

    /// Пердыдущее состояние поляризованного якоря
    int polar_ancor_state_prev = 0;

    void preStep(state_vector_t &Y, double t) override;

    void ode_system(const state_vector_t &Y, state_vector_t &dYdt, double t) override;
};

#endif
