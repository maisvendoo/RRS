#ifndef     EXIT_SIGNAL_H
#define     EXIT_SIGNAL_H

#include    "station-signal.h"

class CombineRelay;
class Relay;

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class TOPOLOGY_EXPORT ExitSignal : public StationSignal
{
public:

    ExitSignal(QObject *parent = nullptr);

    virtual ~ExitSignal() override;

    void step(double t, double dt) override;

private:

    enum
    {
        SS_N_RED = 0,
        SS_N_ALLOW,
        NUM_SRS_NEUTRAL_CONTACTS,

        SS_PLUS_GREEN = 0,
        NUM_SRS_PLUS_CONTACTS,

        SS_MINUS_YELLOW = 0,
        NUM_SRS_MINUS_CONTACTS
    };

    /// Сигнальное реле светофора (с полярным якорем):
    /// при питании положительным напряжением переключает на зелёный,
    /// отрицательным напряжением - жёлтый, без питания - красный
    CombineRelay *semaphore_signal_relay = nullptr;

    enum
    {
        SSR_GREEN = 0,
        SSR_YELLOW,
        NUM_SSR_CONTACTS,
    };

    /// Боковое сигнальное реле (желтый мигающий, если следующий с отклонением по стрелкам)
    Relay *side_signal_relay = nullptr;

    void preStep(double t) override;

    /// Управление цепями питания реле
    void relay_control();

    /// Управление миганием желтого (на предвходном)
    void yellow_blink_control();

    /// Управление состоянием линз
    void lens_control();

    /// Управление состоянием линий АЛСН
    void alsn_control();
};

#endif // EXIT_SIGNAL_H
