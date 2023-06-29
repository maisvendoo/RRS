#ifndef     PNEUMO_ANGLECOCK_H
#define     PNEUMO_ANGLECOCK_H

#include    "brake-device.h"

/*!
 * \class
 * \brief Концевой кран
 */
//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class DEVICE_EXPORT PneumoAngleCock : public BrakeDevice
{
public:

    /// Конструктор
    PneumoAngleCock(QObject *parent = Q_NULLPTR);

    /// Деструктор
    virtual ~PneumoAngleCock();

    /// Задать давление со стороны магистрали
    void setP_pipe(double p);

    /// Задать поток из рукава
    void setQ_hose(double q);

    /// Задать состояние концевого крана: true - открыть, false - закрыть
    void setState(bool opened);

    /// Переключить концевой кран
    void changeState();

    /// Поток в магистраль
    double getQ_pipe() const;

    /// Давление в рукаве
    double getP_hose() const;

    /// Cостояние концевого крана: true - открыт, false - закрыт
    bool getState() const;

protected:

    /// Давление в магистрали
    double p_pipe;

    /// Поток из рукава
    double Q_hose;

    /// Состояние крана
    bool is_opened;

    virtual void preStep(state_vector_t &Y, double t);

    virtual void ode_system(const state_vector_t &Y,
                            state_vector_t &dYdt,
                            double t);

    /// Load configuration
    virtual void load_config(CfgReader &cfg);
};

#endif // PNEUMO_ANGLECOCK_H
