#include    "train-horn.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
TrainHorn::TrainHorn(QObject *parent) : Device(parent)
  , is_svistok(false)
  , is_tifon(false)
  , pFL(0.0)
  , QFL(0.0)
  , p_nom(0.9)
  , k_svistok(5.0e-4)
  , k_tifon(8.0e-4)
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
TrainHorn::~TrainHorn()
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TrainHorn::step(double t, double dt)
{
    // Переопределяем шаг, поскольку выполнение решателя не нужно
    // Нужна только обработка клавиатурного ввода
    stepKeysControl(t, dt);
    stepExternalControl(t, dt);

    // Расчёт коэффициента расхода воздуха в атмосферу при работе звуковых сигналов
    double k = 0.0;
    // Расчёт громкости звуковых сигналов
    int volume_level = static_cast<int>(floor(100.0 * cut(pFL / p_nom, 0.0, 1.0)));

    if (is_svistok)
    {
        k += k_svistok;
        emit soundSetVolume("Svistok", volume_level);
    }
    if (is_tifon)
    {
        k += k_tifon;
        emit soundSetVolume("Tifon", volume_level);
    }

    // Расход воздуха питательной магистрали
    QFL = -k * pFL;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TrainHorn::setSvistokOn(bool state)
{
    if (is_svistok != state)
    {
        if (state)
        {
            emit soundPlay("Svistok");
        }
        else
        {
            emit soundStop("Svistok");
        }
        is_svistok = state;
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool TrainHorn::isSvistok() const
{
    return is_svistok;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TrainHorn::setTifonOn(bool state)
{
    if (is_tifon != state)
    {
        if (state)
        {
            emit soundPlay("Tifon");
        }
        else
        {
            emit soundStop("Tifon");
        }
        is_tifon = state;
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool TrainHorn::isTifon() const
{
    return is_tifon;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TrainHorn::setFLpressure(double value)
{
    pFL = value;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double TrainHorn::getFLflow() const
{
    return QFL;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TrainHorn::ode_system(const state_vector_t &Y,
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
void TrainHorn::load_config(CfgReader &cfg)
{
    QString secName = "Device";

    double tmp = 0.0;
    cfg.getDouble(secName, "p_nom", tmp);
    if (tmp > Physics::ZERO)
        p_nom = tmp;

    cfg.getDouble(secName, "k_svistok", k_svistok);
    cfg.getDouble(secName, "k_tifon", k_tifon);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TrainHorn::stepKeysControl(double t, double dt)
{
    Q_UNUSED(t)
    Q_UNUSED(dt)

    setSvistokOn(getKeyState(KEY_Space));
    setTifonOn(getKeyState(KEY_B));
}
