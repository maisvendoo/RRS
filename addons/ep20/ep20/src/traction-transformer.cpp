#include    "traction-transformer.h"

//------------------------------------------------------------------------------
// Конструктор
//------------------------------------------------------------------------------
TractionTransformer::TractionTransformer(QObject *parent) : Device (parent)
  ,U1(0)
  ,traction_coefficient(15.07)
  ,heating_coil_coefficient(8.33)
  ,winding_coils_heating(0)
{
    std::fill(traction_winding_coils.begin(), traction_winding_coils.end(), 0);
}

//------------------------------------------------------------------------------
// Деструктор
//------------------------------------------------------------------------------
TractionTransformer::~TractionTransformer()
{

}

//------------------------------------------------------------------------------
// Установить напряжение с выхода ГВ
//------------------------------------------------------------------------------
void TractionTransformer::setU1(double U1)
{
    this->U1 = U1;
}

//------------------------------------------------------------------------------
// Получить напряжение тяговой обмотки
//------------------------------------------------------------------------------
double TractionTransformer::getTractionVoltage(size_t i)
{
    if (i < traction_winding_coils.size())
    {
       return traction_winding_coils[i];
    }
    else
    {
        return 0;
    }
}

//------------------------------------------------------------------------------
// Получить напряжение обмотки отопления
//------------------------------------------------------------------------------
double TractionTransformer::getVoltageHeatingCoil()
{
    return winding_coils_heating;
}

//------------------------------------------------------------------------------
// Предварительные шаги
//------------------------------------------------------------------------------
void TractionTransformer::preStep(state_vector_t &Y, double t)
{
    for (size_t i = 0; i < traction_winding_coils.size(); ++i)
    {
        traction_winding_coils[i] = U1 / traction_coefficient;
    }

    winding_coils_heating = U1 / heating_coil_coefficient;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TractionTransformer::ode_system(const state_vector_t &Y, state_vector_t &dYdt, double t)
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TractionTransformer::load_config(CfgReader &cfg)
{

}
