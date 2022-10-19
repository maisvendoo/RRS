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

    /// Задать давление в магистрали
    void setP_pipe(double p);

    /// Задать давление в рукаве
    void setP_hose(double p);

    /// Задать состояние концевого крана: true - открыть, false - закрыть
    void setState(bool opened);

    /// Переключить концевой кран
    void changeState();

    /// Поток в магистраль
    double getQ_pipe() const;

    /// Поток в рукав
    double getQ_hose() const;

    /// Cостояние концевого крана: true - открыт, false - закрыт
    bool getState() const;

protected:

    /// Давление в магистрали
    double p_pipe;

    /// Давление в рукаве
    double p_hose;

    /// Поток в магистраль
    double Q_pipe;

    /// Поток в рукав
    double Q_hose;

    /// Коэффициент потока между рукавом и магистралью
    double k;

    /// Коэффициент потока между рукавом и атмосферой
    double k_atm;

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
