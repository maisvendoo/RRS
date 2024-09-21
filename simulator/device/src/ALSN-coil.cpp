#include    "ALSN-coil.h"

#include    "CfgReader.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
CoilALSN::CoilALSN(QObject *parent) : Device(parent)
{
    name = QString("ALSN");

    input_signals.resize(SIZE_OF_INPUTS);
    output_signals.resize(SIZE_OF_OUTPUTS);

    output_signals[OUTPUT_DIRECTION] = 1.0;
    input_signals[INPUT_FREQUENCY] = 0.0;
    input_signals[INPUT_CODE] = 0.0;
    input_signals[INPUT_NEXT_DISTANCE] = 0.0;
    input_signals[INPUT_LITER_SIZE] = 0.0;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
CoilALSN::~CoilALSN()
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void CoilALSN::setDirection(int direction)
{
    if (direction == 1)
        output_signals[OUTPUT_DIRECTION] = 1.0;
    if (direction == -1)
        output_signals[OUTPUT_DIRECTION] = -1.0;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double CoilALSN::getFrequency() const
{
    if (is_linked)
        return input_signals[INPUT_FREQUENCY];

    // Если нет рельсовой цепи АЛСН, возвращаем 0
    return 0.0;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
ALSN CoilALSN::getCode() const
{
    if (is_linked)
    {
        if (input_signals[INPUT_CODE] == static_cast<double>(ALSN::RED_YELLOW))
            return ALSN::RED_YELLOW;
        if (input_signals[INPUT_CODE] == static_cast<double>(ALSN::YELLOW))
            return ALSN::YELLOW;
        if (input_signals[INPUT_CODE] == static_cast<double>(ALSN::GREEN))
            return ALSN::GREEN;
        return ALSN::NO_CODE;
    }

    // Если нет рельсовой цепи АЛСН, возвращаем 0
    return ALSN::NO_CODE;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double CoilALSN::getNextSignalDistance() const
{
    if (is_linked)
        return input_signals[INPUT_NEXT_DISTANCE];

    // Если нет рельсовой цепи АЛСН, возвращаем 0
    return 0.0;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
QString CoilALSN::getNextSignalLiter() const
{
    return next_liter;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void CoilALSN::step(double t, double dt)
{
    (void) t;
    (void) dt;

    next_liter = "";
    if (is_linked && (input_signals[INPUT_LITER_SIZE]) > 0.0)
    {
        size_t liter_size = min(static_cast<size_t>(input_signals[INPUT_LITER_SIZE]),
                                static_cast<size_t>(INPUT_LITER_MAX_SIZE));
        for (size_t i = 0; i < liter_size; ++i)
        {
            QChar symbol = QChar(static_cast<int>(input_signals[INPUT_LITER_BEGIN + i]));
            next_liter += symbol;
        }
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void CoilALSN::ode_system(const state_vector_t &Y,
                           state_vector_t &dYdt,
                           double t)
{
    (void) Y;
    (void) dYdt;
    (void) t;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void CoilALSN::load_config(CfgReader &cfg)
{
    (void) cfg;
}
