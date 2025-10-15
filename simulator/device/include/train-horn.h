#ifndef     TRAIN_HORN_H
#define     TRAIN_HORN_H

#include    "device.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class DEVICE_EXPORT TrainHorn : public Device
{
public:

    TrainHorn(QObject *parent = nullptr);

    virtual ~TrainHorn();

    void step(double t, double dt);

    /// Задать управляющую клавишу для свистка
    void setKeySymbolSvistok(std::uint16_t key_symbol);

    /// Задать управляющую клавишу для тифона
    void setKeySymbolTifon(std::uint16_t key_symbol);

    /// Задать состояние свистка
    void setSvistokOn(bool state);

    /// Состояние свистка
    bool isSvistok() const;

    /// Задать состояние тифона
    void setTifonOn(bool state);

    /// Состояние тифона
    bool isTifon() const;

    /// Задать давление от питательной магистрали, МПа
    void setFLpressure(double value);

    /// Поток в питательную магистраль
    double getFLflow() const;

    enum {
        NUM_SOUNDS = 2,
        SVISTOK_SOUND = 0,
        TIFON_SOUND = 1
    };
    /// Состояние звука
    sound_state_t getSoundState(size_t idx = SVISTOK_SOUND) const;

    /// Сигнал состояния звука
    float getSoundSignal(size_t idx = SVISTOK_SOUND) const;

protected:

    /// Код управляющей клавиши свистка
    std::uint16_t key_symbol_svistok = KEY_Undefined;

    /// Код управляющей клавиши тифона
    std::uint16_t key_symbol_tifon = KEY_Undefined;

    /// Состояние звуков
    std::array<sound_state_t, NUM_SOUNDS> sounds = {sound_state_t(), sound_state_t()};

    /// Давление питательной магистрали, МПа
    double pFL = 0.0;

    /// Поток в питательную магистраль
    double QFL = 0.0;

    /// Номинальное давление для работы свистка и тифона, МПа
    double p_nom = 0.9;

    /// Коэффициент потока - расхода воздуха на свисток
    double k_svistok = 5.0e-4;

    /// Коэффициент потока - расхода воздуха на тифон
    double k_tifon = 8.0e-4;

    void ode_system(const state_vector_t &Y, state_vector_t &dYdt, double t);

    void load_config(CfgReader &cfg);

    virtual void stepKeysControl(double t, double dt);
};

#endif // TRAIN_HORN_H
