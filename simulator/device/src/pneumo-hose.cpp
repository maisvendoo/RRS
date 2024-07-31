#include    <QLibrary>

#include    "pneumo-hose.h"

#include    "CfgReader.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
PneumoHose::PneumoHose(int key_code, QObject *parent) : Device(parent)
  , keyCode(key_code)
  , is_ref_state_command(false)
{
    name = QString("BP");

    input_signals.resize(HOSE_SIZE_OF_INPUTS);
    output_signals.resize(HOSE_SIZE_OF_OUTPUTS);

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
void PneumoHose::setKeyCode(int key_code)
{
    keyCode = key_code;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void PneumoHose::connect()
{
    output_signals[HOSE_OUTPUT_REF_STATE] = 1.0;
    is_ref_state_command = true;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void PneumoHose::disconnect()
{
    output_signals[HOSE_OUTPUT_REF_STATE] = -1.0;
    is_ref_state_command = true;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool PneumoHose::isConnected() const
{
    return is_linked && (input_signals[HOSE_INPUT_IS_CONNECTED] == 1.0);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void PneumoHose::setPressure(double value)
{
    output_signals[HOSE_OUTPUT_PIPE_PRESSURE] = value;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void PneumoHose::setFlowCoeff(double value)
{
    output_signals[HOSE_OUTPUT_FLOW_COEFF] = value;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void PneumoHose::setLength(double value)
{
    output_signals[HOSE_OUTPUT_LENGTH] = value;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void PneumoHose::setShiftSide(double value)
{
    output_signals[HOSE_OUTPUT_SIDE] = value;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void PneumoHose::setCoord(double value)
{
    output_signals[HOSE_OUTPUT_COORD] = value;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double PneumoHose::getFlow() const
{
    if (is_linked)
        return input_signals[HOSE_INPUT_FLOW_TO_PIPE];

    // Если нет соседнего рукава, то рукав открыт в атмосферу
    return -( output_signals[HOSE_OUTPUT_FLOW_COEFF]
            * output_signals[HOSE_OUTPUT_PIPE_PRESSURE] );
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double PneumoHose::getSideAngle() const
{
    if (isConnected())
        return input_signals[HOSE_INPUT_SIDE_ANGLE];

    return 0.0;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double PneumoHose::getDownAngle() const
{
    if (isConnected())
        return input_signals[HOSE_INPUT_DOWN_ANGLE];

    return 0.0;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
sound_state_t PneumoHose::getSoundState(size_t idx) const
{
    if (idx == PIPE_DRAIN_FLOW_SOUND)
        return atm_flow_sound;
    if (idx == CONNECT_SOUND)
        return sound_state_t(isConnected());
    if (idx == DISCONNECT_SOUND)
        return sound_state_t(!isConnected());
    return sound_state_t();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
float PneumoHose::getSoundSignal(size_t idx) const
{
    if (idx == PIPE_DRAIN_FLOW_SOUND)
        return atm_flow_sound.createSoundSignal();
    if (idx == CONNECT_SOUND)
        return sound_state_t::createSoundSignal(isConnected());
    if (idx == DISCONNECT_SOUND)
        return sound_state_t::createSoundSignal(!isConnected());
    return sound_state_t::createSoundSignal(false);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void PneumoHose::step(double t, double dt)
{
    // Проверяем, вызывались ли методы управления рукавами на данном шаге
    if (is_ref_state_command)
    {
        // Сбрасываем флаг, чтобы обнулить управляющий сигнал на следующем шаге
        is_ref_state_command = false;
    }
    else
    {
        // Если не вызывались, обнуляем управляющий сигнал
        output_signals[HOSE_OUTPUT_REF_STATE] = 0.0f;
    }

    atm_flow_sound.state = !isConnected();
    atm_flow_sound.volume = K_sound * cbrt(getFlow());

    Device::step(t, dt);
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
void PneumoHose::stepKeysControl(double t, double dt)
{
    Q_UNUSED(t)
    Q_UNUSED(dt)

    // Проверяем управляющий сигнал
    if (getKeyState(keyCode))
    {
        if (isShift() && (!isControl()))
        {
            // Соединяем рукава
            connect();
        }
        else
        {
            if (isControl())
                // Разъединяем рукава
                disconnect();
        }
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
    output_signals[HOSE_OUTPUT_FLOW_COEFF] = tmp;

    cfg.getDouble(secName, "K_sound", K_sound);

    tmp = 0.71;
    cfg.getDouble(secName, "Length", tmp);
    if (tmp > Physics::ZERO)
        output_signals[HOSE_OUTPUT_LENGTH] = tmp;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
PneumoHose *loadPneumoHose(QString lib_path)
{
    PneumoHose *hose = nullptr;

    QLibrary lib(lib_path);

    if (lib.load())
    {
        GetPneumoHose getPneumoHose = reinterpret_cast<GetPneumoHose>(lib.resolve("getPneumoHose"));

        if (getPneumoHose)
        {
            hose = getPneumoHose();
        }
    }

    return hose;
}
