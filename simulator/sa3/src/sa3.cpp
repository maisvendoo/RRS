#include    "sa3.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
CouplingSA3::CouplingSA3(QObject *parent) : Coupling(parent)
  , is_lockkeeper_working(false)
{
    name = QString("SA3");
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
CouplingSA3::~CouplingSA3()
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void CouplingSA3::setCouplingOperatingState(double state)
{
    output_signals[COUPL_OUTPUT_REF_STATE] = state;
    if (state == -1.0)
        // Срабатывание замкодержателя на удержание расцепленного состояния
        is_lockkeeper_working = true;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void CouplingSA3::couple()
{
    // Отмена расцепления
    is_lockkeeper_working = false;
    output_signals[COUPL_OUTPUT_REF_STATE] = 0.0;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void CouplingSA3::uncouple()
{
    // Срабатывание замкодержателя на удержание расцепленного состояния
    is_lockkeeper_working = true;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void CouplingSA3::step(double t, double dt)
{
    Q_UNUSED(t)
    Q_UNUSED(dt)
    if (is_lockkeeper_working)
    {
        // Проверяем разведение сцепок
        if (input_signals[COUPL_INPUT_DELTA] > Physics::ZERO)
        {
            // Сцепка снова готова к сцеплению
            output_signals[COUPL_OUTPUT_REF_STATE] = 0.0;
            is_lockkeeper_working = false;
        }
        else
        {
            output_signals[COUPL_OUTPUT_REF_STATE] = -1.0;
        }
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void CouplingSA3::load_config(CfgReader &cfg)
{
    QString secName = "Device";

    cfg.getString(secName, "Name", name);
}

GET_COUPLING(CouplingSA3)
