#ifndef     AUTOMATIC_TRAIN_STOP_H
#define     AUTOMATIC_TRAIN_STOP_H

#include    "brake-device.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class DEVICE_EXPORT AutoTrainStop : public BrakeDevice
{
public:

    AutoTrainStop(QObject* parent = nullptr);

    virtual ~AutoTrainStop() = default;

    /// Задать состояние ключа включения-выключения
    void setKeyOn(bool state) noexcept;

    /// Состояние ключа включения-выключения
    bool isKeyOn() const noexcept;

    /// Задать подачу электропитания
    void setPowered(bool state) noexcept;

    /// Наличие электропитания
    bool isPowered() const noexcept;

    /// Автостопное экстренное торможение
    virtual bool getEmergencyBrakeContact() const;

    /// Задать давление от питательной магистрали
    void setFLpressure(double value) noexcept;

    /// Поток в питательную магистраль
    double getFLflow() const noexcept;

    /// Задать давление от тормозной магистрали
    void setBPpressure(double value) noexcept;

    /// Поток в тормозную магистраль
    double getBPflow() const noexcept;

protected:
    /// Наличие электропитания
    double is_powered = 0.0;

    /// Состояние ключа включения-выключения
    double is_key_on = 0.0;

    /// Давление питательной магистрали
    double pFL = 0.0;
    /// Давление тормозной магистрали
    double pBP = 0.0;

    /// Поток в питательную магистраль
    double QFL = 0.0;
    /// Поток в тормозную магистраль
    double QBP = 0.0;
};

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
using GetAutoTrainStop = AutoTrainStop*(*)();

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
#define GET_AUTO_TRAIN_STOP(ClassName) \
    extern "C" AutoTrainStop* getAutoTrainStop() \
    { \
        return new (ClassName) (); \
    }

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
extern "C" DEVICE_EXPORT AutoTrainStop* loadAutoTrainStop(QString lib_path);

#endif // AUTOMATIC_TRAIN_STOP_H
