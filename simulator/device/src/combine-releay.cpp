#include    <combine-releay.h>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
CombineRelay::CombineRelay(size_t num_neutral_contacts,
                           size_t num_plus_contacts,
                           size_t num_minus_contscts,
                           QObject *parent)
{
    contact.resize(num_neutral_contacts);
    std::fill(contact.begin(), contact.end(), false);

    plus_contact.resize(num_plus_contacts);
    std::fill(plus_contact.begin(), plus_contact.end(), false);

    minus_contact.resize(num_minus_contscts);
    std::fill(minus_contact.begin(), minus_contact.end(), false);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
CombineRelay::~CombineRelay()
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void CombineRelay::ode_system(const state_vector_t &Y, state_vector_t &dYdt, double t)
{
    (void) t;

    // Уравнение для тока нейтрального якоря
    dYdt[0] = (qAbs(U) / R - Y[0]) / T;

    // Уравение для тока поляризованного якоря
    dYdt[1] = (U / R - Y[1]) / T;
}
