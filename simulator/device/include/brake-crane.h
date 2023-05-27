#ifndef     BRAKE_CRANE_H
#define     BRAKE_CRANE_H

#include    "brake-device.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
enum
{
    ER_PRESSURE = 0 ///< Давление в уравнительном резервуаре
};

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class DEVICE_EXPORT BrakeCrane : public BrakeDevice
{
public:

    BrakeCrane(QObject *parent = Q_NULLPTR);

    ~BrakeCrane();

    /// Задать позицию крана
    virtual void setHandlePosition(int &position) = 0;

    /// Наименование текущей позиции крана
    virtual QString getPositionName() const = 0;

    /// Положение рукоятки
    virtual double getHandlePosition() const = 0;

    /// Признак положения перекрыши
    bool isHold() const;

    /// Признак положения торможения
    bool isBrake() const;

    /// Задать зарядное давление
    void setChargePressure(double value);

    /// Задать давление от питательной магистрали
    void setFLpressure(double value);

    /// Поток в питательную магистраль
    double getFLflow() const;

    /// Задать давление от тормозной магистрали
    void setBPpressure(double value);

    /// Поток в тормозную магистраль
    double getBPflow() const;

    /// Задать поток в уравнительный резервуар
    void setERflow(double value);

    /// Давление в уравнительном резервуаре
    double getERpressure() const;

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

    virtual void ode_system(const state_vector_t &Y,
                            state_vector_t &dYdt,
                            double t);
private:

    /// Поток в уравнительный резервуар
    double Qer;
};

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
typedef BrakeCrane* (*GetBrakeCrane)();

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
#define GET_BRAKE_CRANE(ClassName) \
    extern "C" Q_DECL_EXPORT BrakeCrane *getBrakeCrane() \
    { \
        return new (ClassName) (); \
    }

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
extern "C" Q_DECL_EXPORT BrakeCrane *loadBrakeCrane(QString lib_path);

#endif // BRAKE_CRANE_H
