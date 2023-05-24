#ifndef     PNEUMO_ANGLECOCK_H
#define     PNEUMO_ANGLECOCK_H

#include    "device.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class DEVICE_EXPORT PneumoAngleCock : public Device
{
public:

    /// Конструктор
    PneumoAngleCock(QObject *parent = Q_NULLPTR);

    /// Деструктор
    virtual ~PneumoAngleCock();

    /// Закрыть концевой кран
    void close();

    /// Открыть концевой кран
    void open();

    /// Cостояние концевого крана
    bool isOpened() const;

    /// Положение рукоятки: от 0.0 (кран закрыт) до 1.0 (кран открыт)
    double getHandlePosition() const;

    /// Задать давление со стороны магистрали
    void setPipePressure(double value);

    /// Задать поток из рукава
    void setHoseFlow(double value);

    /// Коэффициент перетока через кран
    double getFlowCoeff() const;

    /// Поток в магистраль
    double getFlowToPipe() const;

    /// Давление в рукаве
    double getPressureToHose() const;

protected:

    /// Заданное состояние крана: 0 - закрыт, 1 - открыт
    bool ref_state;

    /// Время переключения концевого крана, с
    double switch_time;

    /// Кран открыт
    bool is_opened;

    /// Давление в магистрали
    double p;

    /// Поток из рукава в магистраль
    double Q;

    /// Коэффициент перетока из рукава в магистраль при открытом кране
    double k;

    /// Коэффициент перетока из рукава в атмосферу при закрытом кране
    double k_atm;

    virtual void ode_system(const state_vector_t &Y, state_vector_t &dYdt, double t);

    virtual void preStep(state_vector_t &Y, double t);

    virtual void load_config(CfgReader &cfg);
};

#endif // PNEUMO_ANGLECOCK_H
