#ifndef     ENTER_SIGNAL_H
#define     ENTER_SIGNAL_H

#include    "station-signal.h"

class Relay;

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class TOPOLOGY_EXPORT EnterSignal : public StationSignal
{
public:

    EnterSignal(QObject *parent = nullptr);

    virtual ~EnterSignal() override;

    void step(double t, double dt) override;

private:

    enum
    {
        MSR_RED = 0,
        MSR_YELLOW,
        MSR_GREEN,
        MSR_BLINK,
        NUM_MSR_CONTACTS,

        SSR_RED = 0,
        SSR_TOP_YELLOW,
        SSR_BOTTOM_YELLOW,
        SSR_SIDE,
        SSR_BLINK,
        NUM_SSR_CONTACTS,

        DSR_TOP_YELLOW = 0,
        DSR_GREEN,
        DSR_BLINK,
        NUM_DSR_CONTACTS,

        BLINK_GREEN = 0,
        BLINK_YELLOW,
        NUM_BLINK_CONTACTS
    };

    /// Главное сигнальное реле:
    /// включено, когда сигнал открыт на маршрут прямо
    Relay *main_signal_relay = nullptr;

    /// Боковое сигнальное реле:
    /// включено, когда сигнал открыт на маршрут с отклонением по стрелкам
    Relay *side_signal_relay = nullptr;

    /// Сигнальное реле сквозного пропуска:
    /// включено, когда следующий светофор открыт
    Relay *direct_signal_relay = nullptr;

    /// Реле мигания верхнего желтого
    Relay *blink_relay = nullptr;

    bool is_yellow_wire_ON = false;

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

#endif // ENTER_SIGNAL_H
