#ifndef     SHIELD_229_H
#define     SHIELD_229_H

#include    "device.h"

//------------------------------------------------------------------------------
// Щиток 229/230 - ряд тумблеров приборной панели помощника машиниста
//------------------------------------------------------------------------------
class Shield_229 final : public Device
{
public:

    enum
    {
        /// Тифон
        TIFON = 0,
        /// Свисток
        WHISTLE = 1,
        /// Обогрев кабины
        CAB_HEAT = 2,
        /// Тусклое освещение кабины
        CAB_LIGHT_LOW = 3,
        /// Яркое освещение кабины
        CAB_LIGHT_HIGH = 4,
        /// Резерв
        RESERV_1 = 5,
        /// Освещение ходовой
        SHASSIS_LIGHT = 6,
        /// Освещение приборов
        DEVICES_LIGHT = 7,
        /// Фонарь левый буферный
        BUFFERLIGHT_L = 8,
        /// Фонарь правый буферный
        BUFFERLIGHT_R = 9,
        /// Резерв
        RESERV_2 = 10,
        /// Проверка АЛСН
        ALSN_CHECK = 11,

        TUMBLERS_NUMBER = 12
    };

    Shield_229(QObject* parent);

    ~Shield_229() = default;

    void setControl(std::set<std::uint16_t>* keys = nullptr,
                    control_signals_t* control_signals = nullptr) override;

    void step(double t, double dt) override;

    /// Задать состояние тумблера
    void setTumblerState(size_t tumbler_idx, bool state);

    /// Состояние тумблера
    bool getTumblerState(size_t tumbler_idx) const;

    float getTumblerSoundSignal(size_t tumbler_idx, size_t idx = Trigger::CHANGE_SOUND);

private:

    /// Тумблеры
    std::array<TriggerControl, TUMBLERS_NUMBER> tumblers;

    void ode_system(const state_vector_t &Y,
                    state_vector_t &dYdt,
                    double t) override;

    void initControl();

    bool isAllTumblersOff() const;
};

#endif // SHIELD_229_H
