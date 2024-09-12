#include    <combine-releay.h>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
CombineRelay::CombineRelay(size_t num_neutral_contacts,
                           size_t num_plus_contacts,
                           size_t num_minus_contscts,
                           QObject *parent) : Relay(num_neutral_contacts, parent)
{
    contact.resize(num_neutral_contacts);
    std::fill(contact.begin(), contact.end(), false);

    plus_contact.resize(num_plus_contacts);
    std::fill(plus_contact.begin(), plus_contact.end(), false);

    minus_contact.resize(num_minus_contscts);
    std::fill(minus_contact.begin(), minus_contact.end(), false);

    polar_hysteresis = new PolarHysteresis(level_off * U_nom / R, level_on * U_nom / R, 0);
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
void CombineRelay::preStep(state_vector_t &Y, double t)
{
    // Обрабатываем нейтральный якорь
    Relay::preStep(Y, t);

    // Обрабатываем поляризованный якорь
    polar_hysteresis->setValue(Y[1]);

    // Если изменилось его состояние
    int state = polar_hysteresis->getState();
    if (polar_ancor_state_prev != state)
    {
        // При переходе в нейтральное положение
        if (state == 0)
        {
            if (polar_ancor_state_prev > 0)
            {
                for (auto pc_it : plus_contact)
                {
                    pc_it = !pc_it;
                }
            }

            if (polar_ancor_state_prev < 0)
            {
                for (auto nc_it : minus_contact)
                {
                    nc_it = !nc_it;
                }
            }
        }

        // при положительной полярности
        if (state > 0)
        {
            for (auto pc_it : plus_contact)
            {
                pc_it = !pc_it;
            }
        }

        // при отрицательной полярности
        if (state < 0)
        {
            for (auto nc_it : minus_contact)
            {
                nc_it = !nc_it;
            }
        }
    }

    polar_ancor_state_prev = state;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void CombineRelay::ode_system(const state_vector_t &Y,
                              state_vector_t &dYdt,
                              double t)
{
    (void) t;

    // Уравнение для тока нейтрального якоря
    dYdt[0] = (qAbs(U) / R - Y[0]) / T;

    // Уравение для тока поляризованного якоря
    dYdt[1] = (U / R - Y[1]) / T;
}
