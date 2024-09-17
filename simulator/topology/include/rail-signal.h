#ifndef     SIGNAL_H
#define     SIGNAL_H

#include    <device.h>
#include    <topology-export.h>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
enum
{
    NUM_LENS = 5,
    RED_LENS = 0,
    YELLOW_LENS = 1,
    GREEN_LENS = 2
};

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
using lens_state_t = std::array<bool, NUM_LENS>;

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class TOPOLOGY_EXPORT Signal : public Device
{
    Q_OBJECT

public:

    Signal(QObject *parent = Q_NULLPTR);

    virtual ~Signal();

    void step(double t, double dt) override;

    QString getLetter() const
    {
        return letter;
    }

    lens_state_t getAllLensState() const
    {
        return lens_state;
    }

    /// Установить занятость блок-участка, предшествующего данному сигналу
    void setBusy(bool is_busy);

signals:

    /// Послать предыдущему световору напряжение линии
    void sendLineVoltage(double U_line);

protected:    

    /// Состояние всех возможных линз
    lens_state_t lens_state;

    /// Литер
    QString letter = "";

    /// Напряжение питания линейного реле
    double U_line = 0.0;

    bool is_busy = false;

    void load_config(CfgReader &cfg) override;

public slots:

    /// Принять от следующего светофора напряжение на линии
    void slotRecvLineVoltage(double U_line);
};

#endif // SIGNAL_H
