#include    "train-horn.h"

#include    "physics.h"

#include    "key-symbols.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
TrainHorn::TrainHorn(QObject *parent) : Device(parent)
{
    setKeySymbolSvistok(KEY_Space);
    setKeySymbolTifon(KEY_B);
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

    // Расчёт коэффициента расхода воздуха в атмосферу при работе звуковых сигналов
    double k = 0.0;
    // Расчёт громкости звуковых сигналов
    float volume_level = std::clamp(static_cast<float>(pFL / p_nom), 0.0f, 1.0f);
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
void TrainHorn::setKeySymbolSvistok(std::uint16_t key_symbol)
{
    key_symbol_svistok = key_symbol;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TrainHorn::setKeySymbolTifon(std::uint16_t key_symbol)
{
    key_symbol_tifon = key_symbol;
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
    (void) Y;
    (void) dYdt;
    (void) t;
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
    (void) t;
    (void) dt;

    setSvistokOn(getKeyState(pressed_keys, key_symbol_svistok));
    setTifonOn(getKeyState(pressed_keys, key_symbol_tifon));
}
