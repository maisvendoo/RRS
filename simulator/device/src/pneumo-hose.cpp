#include    "pneumo-hose.h"

#include    "CfgReader.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
PneumoHose::PneumoHose() : Device()
  , is_ref_state_command(false)
{
    name = QString("tm");

    input_signals.resize(SIZE_OF_INPUTS);
    output_signals.resize(SIZE_OF_OUTPUTS);

    std::fill(input_signals.begin(), input_signals.end(), 0.0);
    std::fill(output_signals.begin(), output_signals.end(), 0.0);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
PneumoHose::~PneumoHose()
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void PneumoHose::connect()
{
    output_signals[HOSE_REF_STATE] = 1.0;
    is_ref_state_command = true;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void PneumoHose::disconnect()
{
    is_ref_state_command = true;
    output_signals[HOSE_REF_STATE] = -1.0;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool PneumoHose::isConnected() const
{
    return is_linked && (input_signals[HOSE_IS_CONNECTED] == 1.0);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void PneumoHose::setPressure(double value)
{
    output_signals[HOSE_PIPE_PRESSURE] = value;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void PneumoHose::setFlowCoeff(double value)
{
    output_signals[HOSE_FLOW_COEFF] = value;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double PneumoHose::getFlow() const
{
    if (is_linked)
        return input_signals[HOSE_FLOW_TO_PIPE];

    // Если нет соседнего рукава, то рукав открыт в атмосферу
    return -output_signals[HOSE_FLOW_COEFF] * output_signals[HOSE_PIPE_PRESSURE];
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void PneumoHose::ode_system(const state_vector_t &Y,
                           state_vector_t &dYdt,
                           double t)
{
    Q_UNUSED(Y)
    Q_UNUSED(dYdt)
    Q_UNUSED(t)
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void PneumoHose::preStep(state_vector_t &Y, double t)
{
    Q_UNUSED(Y)
    Q_UNUSED(t)

    // Проверяем, вызывались ли методы управления рукавами на данном шаге
    if (is_ref_state_command)
    {
        // Сбрасываем флаг, чтобы обнулить управляющий сигнал на следующем шаге
        is_ref_state_command = false;
    }
    else
    {
        // Если не вызывались, обнуляем управляющий сигнал
        output_signals[HOSE_REF_STATE] = 0.0f;
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void PneumoHose::load_config(CfgReader &cfg)
{
    QString secName = "Device";

    cfg.getString(secName, "Name", name);

    double tmp = 1.0;
    cfg.getDouble(secName, "FlowCoefficient", tmp);
    output_signals[HOSE_PIPE_PRESSURE] = tmp;
}
