#ifndef     ABSTRACT_SIGNAL_H
#define     ABSTRACT_SIGNAL_H

#include    "device.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
enum
{
    NUM_LENS = 5
};

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
using lens_state_t = std::array<bool, NUM_LENS>;

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class Signal : public Device
{
public:

    Signal(QObject *parent = Q_NULLPTR);

    ~Signal();

    lens_state_t getAllLensState() const { return lens_state; };

    /// Закрытие сигнала
    virtual void close(bool is_closed);

    /// Задать литер
    void setLiter(const QString &liter) { this->liter = liter; }

    /// Вернуть литер сигнала
    QString getLiter() const { return liter; }

    /// Задать тип сигнала
    void setType(const QString &type) { this->type = type; }

    /// Вернуть тип сигнала
    QString getType() const { return type; }

signals:

    /// Отправка данных о закрытом состоянии сигнала
    void sendClosedState(bool closed_state);

public slots:

    /// Прием состояния (азкрыто/открыто) с предыдущей сигнальной точки
    void slotRecvPreviosState(bool closed_state);

protected:

     /// Признак закрытия предыдущего сигнала
     bool prev_signal_closed;

     /// Литер сигнала
     QString liter;

     /// Тип сигнала
     QString type;

     lens_state_t lens_state;

     virtual void preStep(state_vector_t &Y, double t);

     virtual void ode_system(const state_vector_t &Y,
                             state_vector_t &dYdt,
                             double t);


     virtual void load_config(CfgReader &cfg);

protected slots:

     virtual void slotChangeState();
};

#endif // ABSTRACT_SIGNAL_H
