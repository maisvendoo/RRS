#include    "speedmap.h"

#include    "CfgReader.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
SpeedMap::SpeedMap(QObject *parent) : Device(parent)
{
    name = QString("speedmap");

    input_signals.resize(SIZE_OF_OUTPUTS);
    output_signals.resize(SIZE_OF_INPUTS);

    output_signals[OUTPUT_TRAJECTORY_COORD] = 0.0;
    output_signals[OUTPUT_SEARCH_DIRECTION] = 1.0;
    output_signals[OUTPUT_SEARCH_DISTANCE] = 3000.0;
    input_signals[INPUT_CURRENT_LIMIT] = 300.0;
    input_signals[INPUT_NEXT_LIMIT] = 300.0;
    input_signals[INPUT_NEXT_DISTANCE] = 3000.0;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
SpeedMap::~SpeedMap()
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void SpeedMap::setCoord(double coord)
{
    output_signals[OUTPUT_TRAJECTORY_COORD] = coord;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void SpeedMap::setDirection(int direction)
{
    if (direction == 1)
        output_signals[OUTPUT_SEARCH_DIRECTION] = 1.0;
    if (direction == -1)
        output_signals[OUTPUT_SEARCH_DIRECTION] = -1.0;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void SpeedMap::setNextSearchDistance(double distance)
{
    output_signals[OUTPUT_SEARCH_DISTANCE] = distance;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double SpeedMap::getCurrentLimit() const
{
    if (is_linked)
        return input_signals[INPUT_CURRENT_LIMIT];

    // Если нет карты скоростей, возвращаем 300
    return 300.0;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double SpeedMap::getNextLimit() const
{
    if (is_linked)
        return input_signals[INPUT_NEXT_LIMIT];

    // Если нет карты скоростей, возвращаем 300
    return 300.0;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double SpeedMap::getNextLimitDistance() const
{
    if (is_linked)
        return input_signals[INPUT_NEXT_DISTANCE];

    // Если нет карты скоростей, возвращаем 3000
    return 3000.0;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void SpeedMap::step(double t, double dt)
{
    (void) t;
    (void) dt;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void SpeedMap::ode_system(const state_vector_t &Y,
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
void SpeedMap::load_config(CfgReader &cfg)
{
    QString secName = "Device";

    cfg.getString(secName, "Name", name);

    double tmp = 3000.0;
    cfg.getDouble(secName, "SearchDistance", tmp);
    output_signals[OUTPUT_SEARCH_DISTANCE] = tmp;
}
