#include    <QLibrary>

#include    "coupling.h"

#include    "CfgReader.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
Coupling::Coupling(QObject *parent) : Device(parent)
  , is_ref_state_command(false)
{
    name = QString("Coupling");

    input_signals.resize(COUPL_SIZE_OF_INPUTS);
    output_signals.resize(COUPL_SIZE_OF_OUTPUTS);

    std::fill(input_signals.begin(), input_signals.end(), 0.0);
    std::fill(output_signals.begin(), output_signals.end(), 0.0);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
Coupling::~Coupling()
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Coupling::couple()
{
    output_signals[COUPL_OUTPUT_REF_STATE] = 1.0;
    is_ref_state_command = true;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Coupling::uncouple()
{
    output_signals[COUPL_OUTPUT_REF_STATE] = -1.0;
    is_ref_state_command = true;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool Coupling::isCoupled() const
{
    return is_linked && (input_signals[COUPL_INPUT_IS_CONNECTED] == 1.0);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Coupling::setCoord(double value)
{
    output_signals[COUPL_OUTPUT_COORD] = value;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Coupling::setVelocity(double value)
{
    output_signals[COUPL_OUTPUT_VELOCITY] = value;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double Coupling::getForce() const
{
    if (is_linked)
        return input_signals[COUPL_INPUT_FORCE];

    // Если нет соседней сцепки, нет тягового усилия
    return 0.0;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Coupling::step(double t, double dt)
{
    // Проверяем, вызывались ли методы управления сцепкой на данном шаге
    if (is_ref_state_command)
    {
        // Сбрасываем флаг, чтобы обнулить управляющий сигнал на следующем шаге
        is_ref_state_command = false;
    }
    else
    {
        // Если не вызывались, обнуляем управляющий сигнал
        output_signals[COUPL_OUTPUT_REF_STATE] = 0.0f;
    }

    Device::step(t, dt);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Coupling::ode_system(const state_vector_t &Y,
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
void Coupling::load_config(CfgReader &cfg)
{
    QString secName = "Device";

    cfg.getString(secName, "Name", name);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
Coupling *loadCoupling(QString lib_path)
{
    Coupling *coupling = nullptr;

    QLibrary lib(lib_path);

    if (lib.load())
    {
        GetCoupling getCoupling = reinterpret_cast<GetCoupling>(lib.resolve("getCoupling"));

        if (getCoupling)
        {
            coupling = getCoupling();
        }
    }

    return coupling;
}
