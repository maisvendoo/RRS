#include    "starter-generator.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
StarterGenerator::StarterGenerator(QObject *parent) : Device(parent)
  , Ua(0.0)
  , Uf(0.0)
  , Ia(0.0)
  , If(0.0)
  , Ra(0.039)
  , Rf(6.4)
  , Ta(0.1)
  , Tf(0.1)
  , omega(0.0)
  , M(0.0)
  , is_motor(true)
  , In(0.0)
  , U(0.0)
  , is_started(false)
  , switch_timer(new Timer)

{
    connect(switch_timer, &Timer::process, this, &StarterGenerator::slotSwitchMode);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
StarterGenerator::~StarterGenerator()
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void StarterGenerator::init(QString magnetic_char_file)
{
    magnetic_char.load(magnetic_char_file.toStdString());
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void StarterGenerator::step(double t, double dt)
{
    switch_timer->step(t, dt);
    Device::step(t, dt);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void StarterGenerator::setMotorMode(bool is_motor)
{
    if (is_motor != this->is_motor)
        switch_timer->start();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void StarterGenerator::setLoadCurrent(double In)
{
    this->In = In;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void StarterGenerator::setOmega(double omega)
{
    this->omega = omega;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double StarterGenerator::getVoltage() const
{
    return U;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double StarterGenerator::getTorque()
{
    if (is_motor)
    {
        return cPhi(If) * Ia;
    }
    else
    {
        return -cPhi(If) * Ia;
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void StarterGenerator::preStep(state_vector_t &Y, double t)
{
    Q_UNUSED(t)

    if (is_motor)
    {
        preStep_motor(Y, t);
    }
    else
    {
        preStep_generator(Y, t);
    }

    if ( (Ua >= 62.0) && !is_started)
    {
        emit soundPlay("Starter");
        is_started = true;
    }

    if ( (Ua < 62.0) && is_started)
    {
        emit soundStop("Starter");
        is_started = false;
    }

    emit soundSetPitch("Starter", static_cast<float>(omega / 66.0));
    emit soundSetVolume("Starter", static_cast<int>(Y[0] * 100.0 / 1600.0));

    DebugMsg = QString("Uсг: %1 Iсг: %2 omg: %3")
            .arg(Ua, 5, 'f', 1)
            .arg(Ia, 6, 'f', 1)
            .arg(omega, 6, 'f', 1);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void StarterGenerator::ode_system(const state_vector_t &Y,
                                  state_vector_t &dYdt,
                                  double t)
{
    if (is_motor)
    {
        ode_system_motor(Y, dYdt, t);
    }
    else
    {
        ode_system_generator(Y, dYdt, t);
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void StarterGenerator::load_config(CfgReader &cfg)
{
    QString secName = "Device";

    cfg.getDouble(secName, "Ra", Ra);
    cfg.getDouble(secName, "Rf", Rf);
    cfg.getDouble(secName, "Ta", Ta);
    cfg.getDouble(secName, "Tf", Tf);

    double switch_delay = 0.0;
    cfg.getDouble(secName, "SwitchDelay", switch_delay);

    switch_timer->setTimeout(switch_delay);
    switch_timer->firstProcess(false);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void StarterGenerator::preStep_motor(state_vector_t &Y, double t)
{
    Q_UNUSED(t)

    Y[1] = Y[0];

    Ia = Y[0];
    If = Y[1];

    U = 0.0;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void StarterGenerator::preStep_generator(state_vector_t &Y, double t)
{
    Q_UNUSED(t)

    Ia = Y[0] = In;
    If = Y[1];

    U = cPhi(Y[1]) * omega - Ra * In;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void StarterGenerator::ode_system_motor(const state_vector_t &Y,
                                        state_vector_t &dYdt,
                                        double t)
{
    Q_UNUSED(t)

    dYdt[0] = (Ua - Ra * Y[0] - cPhi(Y[0]) * omega) / Ta / Ra;
    dYdt[1] = 0.0;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void StarterGenerator::ode_system_generator(const state_vector_t &Y,
                                            state_vector_t &dYdt,
                                            double t)
{
    Q_UNUSED(t)

    dYdt[0] = 0.0;
    dYdt[1] = (Uf - Rf * Y[1]) / Tf / Rf;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double StarterGenerator::cPhi(double If)
{
    return 1.8 * magnetic_char.getValue(If);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void StarterGenerator::slotSwitchMode()
{
    is_motor = !is_motor;
    switch_timer->stop();
}
