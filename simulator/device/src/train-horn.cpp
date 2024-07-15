#include    "train-horn.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
TrainHorn::TrainHorn(QObject *parent) : Device(parent)
    , pFL(0.0)
    , QFL(0.0)
    , p_nom(0.9)
    , k_svistok(5.0e-4)
    , k_tifon(8.0e-4)
{
    std::fill(sounds.begin(), sounds.end(), sound_state_t());
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
    float volume_level = cut(static_cast<float>(pFL / p_nom), 0.0f, 1.0f);
    sounds[SVISTOK_SOUND].volume = volume_level;
    sounds[TIFON_SOUND].volume = volume_level;

    if (sounds[SVISTOK_SOUND].state)
    {
        k += k_svistok;
    }
    if (sounds[TIFON_SOUND].state)
    {
        k += k_tifon;
    }

    // Расход воздуха питательной магистрали
    QFL = -k * pFL;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TrainHorn::setSvistokOn(bool state)
{
    sounds[SVISTOK_SOUND].state = state;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool TrainHorn::isSvistok() const
{
    return sounds[SVISTOK_SOUND].state;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TrainHorn::setTifonOn(bool state)
{
    sounds[TIFON_SOUND].state = state;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool TrainHorn::isTifon() const
{
    return sounds[TIFON_SOUND].state;
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
sound_state_t TrainHorn::getSoundState(size_t idx) const
{
    if (idx < sounds.size())
        return sounds[idx];
    return Device::getSoundState();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
float TrainHorn::getSoundSignal(size_t idx) const
{
    if (idx < sounds.size())
        return sounds[idx].createSoundSignal();
    return Device::getSoundSignal();
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
