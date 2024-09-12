#ifndef     COMBINE_RELAY_H
#define     COMBINE_RELAY_H

#include    <relay.h>

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

private:

    /// Контакты, активируемые при положительной полярности
    std::vector<bool> plus_contact;

    /// Контакты, активируемые при отрицательной полярности
    std::vector<bool> minus_contact;

    void ode_system(const state_vector_t &Y, state_vector_t &dYdt, double t) override;
};

#endif
