#include    "pantograph.h"

//------------------------------------------------------------------------------
// Конструктор класса
//------------------------------------------------------------------------------
Pantograph::Pantograph(QObject *parent) : Device(parent)
    , Uks(0.0)
    , Uout(0.0)
    , max_height(1.0)
    , motion_time(4.0)
    , current_kind_in(0)
    , current_kind_out(0)
    , is_up(false)
    , is_down(true)
{

}

//------------------------------------------------------------------------------
// Конструктор класса
//------------------------------------------------------------------------------
Pantograph::~Pantograph()
{

}

//------------------------------------------------------------------------------
// Устновить состояние
//------------------------------------------------------------------------------
void Pantograph::setState(bool state)
{
    if (state)
    {
        ref_state.set();
    }
    else
    {
        ref_state.reset();
    }
}

//------------------------------------------------------------------------------
// Устновить напряжение в КС
//------------------------------------------------------------------------------
void Pantograph::setUks(double Uks)
{
    this->Uks = Uks;
}

//------------------------------------------------------------------------------
// Установить входное значение рода тока
//------------------------------------------------------------------------------
void Pantograph::setCurrentKindIn(int kindCurrent)
{
    this->current_kind_in = kindCurrent;
}

//----------------------------------------------------------------------------
// Получить высоту
//----------------------------------------------------------------------------
double Pantograph::getHeight() const
{
    return getY(0);
}

//----------------------------------------------------------------------------
// Получить напряжение на выходе
//----------------------------------------------------------------------------
double Pantograph::getUout() const
{
    return Uout;
}

//----------------------------------------------------------------------------
// Получить выходное значение рода тока
//----------------------------------------------------------------------------
int Pantograph::getCurrentKindOut() const
{
    return current_kind_out;
}

//----------------------------------------------------------------------------
// Получить состояние поднятого поднятого токоприемника
//----------------------------------------------------------------------------
bool Pantograph::isUp() const
{
    return is_up;
}

//----------------------------------------------------------------------------
// Получить состояние опущенного поднятого токоприемника
//----------------------------------------------------------------------------
bool Pantograph::isDown() const
{
    return is_down;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
sound_state_t Pantograph::getSoundState(size_t idx) const
{
    return ref_state.getSoundState(idx);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
float Pantograph::getSoundSignal(size_t idx) const
{
    return ref_state.getSoundSignal(idx);
}

//----------------------------------------------------------------------------
// Предварительный шаг
//----------------------------------------------------------------------------
void Pantograph::preStep(state_vector_t &Y, double t)
{
    Q_UNUSED(t)
    Q_UNUSED(Y)

    double level = 0.999;

    Uout = Uks * hs_p(Y[0] - level * max_height);

    current_kind_out = current_kind_in * static_cast<int>(hs_p(Y[0] - level * max_height));

    is_up = static_cast<bool>(hs_p(Y[0] - level * max_height));

    is_down = static_cast<bool>(hs_n(Y[0] - (1.0 - level) * max_height));
}

//----------------------------------------------------------------------------
// Вычисление выходного напряжения, рода тока!
//----------------------------------------------------------------------------
void Pantograph::ode_system(const state_vector_t &Y, state_vector_t &dYdt, double t)
{
    Q_UNUSED(t)

    double ref_height = static_cast<double>(ref_state.getState()) * max_height;

    dYdt[0] = 3.0 * (ref_height - Y[0]) / motion_time;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Pantograph::load_config(CfgReader &cfg)
{
    Q_UNUSED(cfg)

    QString secName = "Device";

    cfg.getDouble(secName, "MaxHeight", max_height);
    cfg.getDouble(secName, "MotionTime", motion_time);
}
