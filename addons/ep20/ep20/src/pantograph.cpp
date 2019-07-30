#include    "pantograph.h"

//------------------------------------------------------------------------------
// Конструктор класса
//------------------------------------------------------------------------------
Pantograph::Pantograph(QObject *parent) : Device(parent)
  , Uks(0.0)
  , Uout(0.0)
  , state(false)
  , old_state(false)
  , max_height(1.0)
  , motion_time(4.0)
  , RTout(0)

{
//    load_config(config_path);
}



Pantograph::~Pantograph()
{

}


void Pantograph::setState(bool state)
{
    old_state = this->state;
    this->state = state;
}


void Pantograph::preStep(state_vector_t &Y, double t)
{

}

void Pantograph::ode_system(const state_vector_t &Y,
                            state_vector_t &dYdt,
                            double t)
{
    Q_UNUSED(t)

    double ref_height = static_cast<double>(state) * max_height;


}


void Pantograph::load_config(CfgReader &cfg)
{
//    Q_UNUSED(cfg)
}
