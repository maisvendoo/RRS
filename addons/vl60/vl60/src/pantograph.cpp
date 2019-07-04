#include    "pantograph.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
Pantograph::Pantograph(QString config_path, QObject *parent) : Device(parent)
  , Uks(0.0)
  , Uout(0.0)
  , state(false)
  , old_state(false)
  , max_height(1.0)
  , motion_time(6.0)

{
    load_config(config_path);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
Pantograph::~Pantograph()
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Pantograph::setState(bool state)
{
    old_state = this->state;
    this->state = state;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Pantograph::setUks(double Uks)
{
    this->Uks = Uks;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double Pantograph::getHeight() const
{
    return getY(0);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double Pantograph::getUout() const
{
    return Uout;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Pantograph::preStep(state_vector_t &Y, double t)
{
    Q_UNUSED(t)
    Q_UNUSED(Y)

    if (state != old_state)
    {
        if (state)
            emit soundPlay("Pantograph_Up");
        else
            emit soundPlay("Pantograph_Down");
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Pantograph::ode_system(const state_vector_t &Y,
                            state_vector_t &dYdt,
                            double t)
{
    Q_UNUSED(t)

    double ref_height = static_cast<double>(state) * max_height;

    Uout = Uks * hs_p(Y[0] - 0.95 * max_height);

    dYdt[0] = 3.0 * (ref_height - Y[0]) / motion_time;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Pantograph::load_config(CfgReader &cfg)
{
    Q_UNUSED(cfg)
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Pantograph::load_config(QString cfg_path)
{
    CfgReader cfg;


    if (cfg.load(cfg_path))
    {
        QString secName = "Device";

        cfg.getDouble(secName, "MaxHeight", max_height);
        cfg.getDouble(secName, "MotionTime", motion_time);
    }
}
