#include    "pneumo-reducer-panel.h"

#include    <QDir>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
PneumoReducerPanel::PneumoReducerPanel(QObject *parent) : Device(parent)
  , p_kp1(0.15)
  , p_kp5(0.7)
  , QFL(0.0)
  , Q1(0.0)
  , Q5(0.0)
  , kp1(new PneumoReducer(p_kp1))
  , kp5(new PneumoReducer(p_kp5))
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
PneumoReducerPanel::~PneumoReducerPanel()
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void PneumoReducerPanel::setFLpressure(double value)
{
    kp1->setInputPressure(value);
    kp5->setInputPressure(value);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double PneumoReducerPanel::getFLflow() const
{
    return QFL;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void PneumoReducerPanel::setPressure1(double value)
{
    kp1->setOutPressure(value);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void PneumoReducerPanel::setPressure5(double value)
{
    kp5->setOutPressure(value);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void PneumoReducerPanel::step(double t, double dt)
{
    QFL = kp1->getInputFlow() + kp5->getInputFlow();

    Q1 = kp1->getOutFlow();
    Q5 = kp5->getOutFlow();

    kp1->step(t, dt);
    kp5->step(t, dt);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void PneumoReducerPanel::ode_system(const state_vector_t &Y,
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
void PneumoReducerPanel::load_config(CfgReader &cfg)
{
    kp1->read_config("pneumo-reducer");
    kp5->read_config("pneumo-reducer");

    QString secName = "Device";

    cfg.getDouble(secName, "p_kp1", p_kp1);
    cfg.getDouble(secName, "p_kp5", p_kp5);

    kp1->setRefPressure(p_kp1);
    kp5->setRefPressure(p_kp5);
}
