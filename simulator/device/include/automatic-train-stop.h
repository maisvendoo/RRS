#ifndef     AUTOMATIC_TRAIN_STOP_H
#define     AUTOMATIC_TRAIN_STOP_H

#include    "brake-device.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class DEVICE_EXPORT AutoTrainStop : public BrakeDevice
{
public:

    AutoTrainStop(QObject *parent = Q_NULLPTR);

    virtual ~AutoTrainStop();

    /// Задать состояние ключа включения-выключения
    void setKeyOn(bool state);

    /// Состояние ключа включения-выключения
    bool isKeyOn() const;

    /// Задать подачу электропитания
    void setPowered(bool state);

    /// Наличие электропитания
    bool isPowered() const;

    /// Автостопное экстренное торможение
    virtual bool getEmergencyBrakeContact() const;

    /// Задать давление от питательной магистрали
    void setFLpressure(double value);

    /// Поток в питательную магистраль
    double getFLflow() const;

    /// Задать давление от тормозной магистрали
    void setBPpressure(double value);

    /// Поток в тормозную магистраль
    double getBPflow() const;

protected:

    /// Наличие электропитания
    double is_powered;

    /// Состояние ключа включения-выключения
    double is_key_on;

    /// Давление питательной магистрали
    double pFL;
    /// Давление тормозной магистрали
    double pBP;

    /// Поток в питательную магистраль
    double QFL;
    /// Поток в тормозную магистраль
    double QBP;
};

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
typedef AutoTrainStop* (*GetAutoTrainStop)();

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
#define GET_AUTO_TRAIN_STOP(ClassName) \
    extern "C" Q_DECL_EXPORT AutoTrainStop *getAutoTrainStop() \
    { \
        return new (ClassName) (); \
    }

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
extern "C" Q_DECL_EXPORT AutoTrainStop *loadAutoTrainStop(QString lib_path);

#endif // AUTOMATIC_TRAIN_STOP_H
