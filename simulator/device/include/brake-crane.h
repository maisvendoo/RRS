#ifndef     BRAKE_CRANE_H
#define     BRAKE_CRANE_H

#include    "brake-device.h"

static constexpr int ER_PRESSURE = 0; ///< Давление в уравнительном резервуаре

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class DEVICE_EXPORT BrakeCrane : public BrakeDevice
{
public:
    BrakeCrane(QObject* parent = nullptr);

    virtual ~BrakeCrane() = default;

    /// Задать позицию крана
    virtual void setHandlePosition(int position) = 0;

    /// Наименование текущей позиции крана
    virtual QString getPositionName() const = 0;

    /// Положение рукоятки
    virtual double getHandlePosition() const = 0;

    /// Признак положения перекрыши
    bool isHold() const noexcept;

    /// Признак положения торможения
    bool isBrake() const noexcept;

    /// Задать зарядное давление
    void setChargePressure(double value) noexcept;

    /// Задать давление от питательной магистрали
    void setFLpressure(double value) noexcept;

    /// Поток в питательную магистраль
    double getFLflow() const noexcept;

    /// Задать давление от тормозной магистрали
    void setBPpressure(double value) noexcept;

    /// Поток в тормозную магистраль
    double getBPflow() const noexcept;

    /// Задать поток в уравнительный резервуар
    void setERflow(double value) noexcept;

    /// Давление в уравнительном резервуаре
    double getERpressure() const;

    enum {
        NUM_SOUNDS = 6,
        CHANGE_POS_SOUND = 0,   ///< Звук переключения
        ER_STAB_SOUND = 1,      ///< Звук стабилизатора уравнительного резервуара
        ER_FILL_FLOW_SOUND = 2, ///< Звук наполнения уравнительного резервуара
        ER_DRAIN_FLOW_SOUND = 3,///< Звук опорожнения уравнительного резервуара
        BP_FILL_FLOW_SOUND = 4, ///< Звук наполнения тормозной магистрали
        BP_DRAIN_FLOW_SOUND = 5 ///< Звук опорожнения тормозной магистрали
    };
    /// Состояние звука
    virtual sound_state_t getSoundState(size_t idx = CHANGE_POS_SOUND) const;

    /// Сигнал состояния звука
    virtual float getSoundSignal(size_t idx = CHANGE_POS_SOUND) const;

protected:

    /// Признак положения перекрыши
    bool is_hold;

    /// Признак положения торможения
    bool is_brake;

    /// Зарядное давление
    double p0;

    /// Объём уравнительного резервуара
    double Ver;

    double pFL;
    double pBP;

    double QFL;
    double QBP;

    /// Состояние звуков
    std::vector<sound_state_t> sounds;

    virtual void ode_system(const state_vector_t &Y,
                            state_vector_t &dYdt,
                            double t);
private:

    /// Поток в уравнительный резервуар
    double Qer;
};

#endif // BRAKE_CRANE_H
