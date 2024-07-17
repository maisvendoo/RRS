#ifndef     BRAKE_LOCK_H
#define     BRAKE_LOCK_H

#include    "brake-device.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class DEVICE_EXPORT BrakeLock : public BrakeDevice
{
public:

    BrakeLock(QObject *parent = Q_NULLPTR);

    ~BrakeLock();

    void init(double pBP, double pFL);

    // Блокировочное устройство
    /// Задать состояние блокировочного устройства:
    /// false - заблокировать, true - разблокировать;
    /// возвращает false, если вращение ручки заблокировано стопором
    bool setState(bool state);

    /// Состояние блокировочного устройства:
    /// false - заблокировано, true - разблокировано
    bool isUnlocked() const;

    /// Состояние стопора рукоятки блокировочного устройства:
    /// false - вращение разблокировано, true - вращение заблокировано
    bool isMainHandleLocked() const;

    /// Положение рукоятки блокировочного устройства:
    /// 1.0 - заблокированное положение, 0.0 - разблокированное положение
    double getMainHandlePosition() const;

    // Комбинированный кран
    /// Задать позицию комбинированного крана:
    /// -1 - положение двойной тяги, 0 - поездное положение, 1 - экстренное торможение
    void setCombineCranePosition(int pos);

    /// Положение рукоятки комбинированного крана:
    /// -1.0 - положение двойной тяги, 0.0 - поездное положение, 1.0 - экстренное торможение
    double getCombCranePosition() const;

    // Взаимодействие оборудования, подключаемого через данное блокировочное устройство
    // Взаимодействие кранов и питательной магистрали
    /// Задать давление от питательной магистрали
    void setFLpressure(double value);

    /// Давление питательной магистрали к крану
    double getCraneFLpressure() const;

    /// Задать поток из крана в питательную магистраль
    void setCraneFLflow(double value);

    /// Поток в питательную магистраль
    double getFLflow() const;

    // Взаимодействие кранов и тормозной магистрали
    /// Задать давление от тормозной магистрали
    void setBPpressure(double value);

    /// Давление тормозной магистрали к крану
    double getCraneBPpressure() const;

    /// Задать поток из крана в тормозную магистраль
    void setCraneBPflow(double value);

    /// Поток в тормозную магистраль
    double getBPflow() const;

    // Взаимодействие кранов и магистрали тормозных цилиндров
    /// Задать давление от магистрали тормозных цилиндров
    void setBCpressure(double value);

    /// Давление магистрали тормозных цилиндров к крану
    double getCraneBCpressure() const;

    /// Задать поток из крана в магистраль тормозных цилиндров
    void setCraneBCflow(double value);

    /// Поток в магистраль тормозных цилиндров
    double getBCflow() const;

    enum {
        NUM_SOUNDS = 3,
        CHANGE_LOCK_POS_SOUND = 0,  ///< Звук переключения блокировки
        CHANGE_COMB_POS_SOUND = 1,  ///< Звук переключения комбинированного крана
        BP_DRAIN_FLOW_SOUND = 2,    ///< Звук опорожнения тормозной магистрали
    };
    /// Состояние звука
    virtual sound_state_t getSoundState(size_t idx = CHANGE_LOCK_POS_SOUND) const;

    /// Сигнал состояния звука
    virtual float getSoundSignal(size_t idx = CHANGE_LOCK_POS_SOUND) const;

private:

    /// Состояние блокировочного устройства:
    /// false - заблокировано, true - разблокировано
    bool    state;

    /// Признак стопора рукоятки блокировочного устройства
    bool    is_handle_locked;

    /// Давление в тормозной магистрали, блокирующее рукоятку устройства
    double  p_lock;

    /// Положение рукоятки комбинированного крана:
    /// -1 - положение двойной тяги, 0 - поездное положение, 1 - экстренное торможение
    int    comb_crane_pos;

    double pFL;
    double pBP;
    double pBC;

    double QFL;
    double QBP;
    double QBC;

    /// Условный объём труб между кранами и блокировочным устройством
    double  V0;

    /// Коэффициент потока - разрядки ТМ при экстренном торможении
    double K_emergency;

    /// Коэффициент громкости озвучки к потоку разрядки ТМ комбинированным краном
    double K_sound;

    Timer   *incCombCrane;
    Timer   *decCombCrane;

    /// Состояние звуков
    std::array<sound_state_t, NUM_SOUNDS> sounds;

    void preStep(state_vector_t &Y, double t);

    void ode_system(const state_vector_t &Y, state_vector_t &dYdt, double t);

    void load_config(CfgReader &cfg);

    void stepKeysControl(double t, double dt);

    void combCraneInc();

    void combCraneDec();

};

#endif // BRAKE_LOCK_H
