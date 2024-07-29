#include    "sanding-system.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
SandingSystem::SandingSystem(QObject *parent) : Device(parent)
  , is_sand(false)
  , sand_mass_max(2000.0)
  , sand_flow_nom(2.0)
  , sand_flow(0.0)
  , pFL(0.0)
  , QFL(0.0)
  , p_nom(0.9)
  , k_air(5.0e-4)
  , k_friction(1.3)
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
SandingSystem::~SandingSystem()
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void SandingSystem::setSandDeliveryOn(bool state)
{
    if (is_sand != state) // Заготовка под возможность звука
    {
        if (state)
        {
            //emit soundPlay("Sand");
        }
        else
        {
            //emit soundStop("Sand");
        }
        is_sand = state;
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool SandingSystem::isSandDelivery() const
{
    return is_sand;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double SandingSystem::getSandFlow() const
{
    return sand_flow;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double SandingSystem::getWheelRailFrictionCoeff(double current_coeff) const
{
    return (1.0 + (k_friction - 1.0) * (sand_flow / sand_flow_nom)) * current_coeff;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void SandingSystem::setSandMassMax(double value)
{
    sand_mass_max = value;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void SandingSystem::setSandMass(double value)
{
    setY(0, value / sand_mass_max);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void SandingSystem::setSandLevel(double level)
{
    setY(0, level);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double SandingSystem::getSandMass() const
{
    return getY(0) * sand_mass_max;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double SandingSystem::getSandLevel() const
{
    return getY(0);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void SandingSystem::setFLpressure(double value)
{
    pFL = value;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double SandingSystem::getFLflow() const
{
    return QFL;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
sound_state_t SandingSystem::getSoundState(size_t idx) const
{
    (void) idx;
    return sound_state_t(is_sand);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
float SandingSystem::getSoundSignal(size_t idx) const
{
    (void) idx;
    return sound_state_t::createSoundSignal(is_sand);
}
//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void SandingSystem::ode_system(const state_vector_t &Y,
                           state_vector_t &dYdt,
                           double t)
{
    (void) t;

    if (is_sand && (pFL > Physics::ZERO))
    {
        // Расход воздуха
        QFL = -k_air * pFL;

        // Проверяем наличие песка и давление в питательной магистрали
        sand_flow = hs_p(Y[0]) * (pFL / p_nom) * sand_flow_nom;

        // Применяем расход песка к уровню в бункере
        // Не забыв пересчитать кг/мин в кг/сек
        dYdt[0] = -sand_flow / sand_mass_max / 60.0;
    }
    else
    {
        QFL = 0.0;
        sand_flow = 0.0;
        dYdt[0] = 0.0;
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void SandingSystem::load_config(CfgReader &cfg)
{
    QString secName = "Device";

    double tmp = 0.0;
    cfg.getDouble(secName, "sand_mass_max", tmp);
    if (tmp > Physics::ZERO)
        sand_mass_max = tmp;

    tmp = 0.0;
    cfg.getDouble(secName, "sand_flow_nom", tmp);
    if (tmp > Physics::ZERO)
        sand_flow_nom = tmp;

    tmp = 0.0;
    cfg.getDouble(secName, "p_nom", tmp);
    if (tmp > Physics::ZERO)
        p_nom = tmp;

    cfg.getDouble(secName, "k_air", k_air);
    cfg.getDouble(secName, "k_friction", k_friction);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void SandingSystem::stepKeysControl(double t, double dt)
{
    Q_UNUSED(t)
    Q_UNUSED(dt)

    // Активация песочницы прикручена к клавише Del
    setSandDeliveryOn(getKeyState(KEY_Delete));
}
