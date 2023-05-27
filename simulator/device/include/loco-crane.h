#ifndef     LOCO_CRANE_H
#define     LOCO_CRANE_H

#include    "brake-device.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class DEVICE_EXPORT LocoCrane : public BrakeDevice
{
public:

    LocoCrane(QObject *parent = Q_NULLPTR);

    virtual ~LocoCrane();

    /// Задать позицию крана
    virtual void setHandlePosition(double position);

    /// Задать отпуск тормозов на локомотиве
    void release(bool is_release);

    /// Положение рукоятки (вращение)
    virtual double getHandlePosition() const = 0;

    /// Положение рукоятки (подъём по вертикальной оси)
    virtual double getHandleShift() const = 0;

    /// Задать давление от питательной магистрали
    void setFLpressure(double value);

    /// Поток в питательную магистраль
    double getFLflow() const;

    /// Задать давление от магистрали тормозных цилиндров
    void setBCpressure(double value);

    /// Поток в магистраль тормозных цилиндров
    double getBCflow() const;

    /// Задать давление от импульсной магистрали (давление от воздухораспределителя)
    void setILpressure(double value);

    /// Поток в импульсную магистраль
    double getILflow() const;

    /// Моделирование звуков
    virtual void stepSound();

protected:

    /// Положение рукоятки
    double pos;

    /// Признак отпуска тормозов на локомотиве
    double is_release;

    double pFL;
    double pBC;
    double pIL;

    double QFL;
    double QBC;
    double QIL;
};

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
typedef LocoCrane* (*GetLocoCrane)();

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
#define GET_LOCO_CRANE(ClassName) \
    extern "C" Q_DECL_EXPORT LocoCrane *getLocoCrane() \
    { \
        return new (ClassName) (); \
    }

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
extern "C" Q_DECL_EXPORT LocoCrane *loadLocoCrane(QString lib_path);

#endif // LOCO_CRANE_H
