#ifndef     ENTER_SIGNAL_H
#define     ENTER_SIGNAL_H

#include    "station-signal.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class TOPOLOGY_EXPORT EnterSignal : public StationSignal
{
public:

    EnterSignal(QObject *parent = nullptr);

    ~EnterSignal();

    void step(double t, double dt) override;

private:

    enum
    {
        NUM_MSR_CONTACTS = 3,
        NUM_SSR_CONTACTS = 5,
        NUM_DSR_CONTACTS = 3,
        NUM_BLINK_CONTACTS = 2,

        MSR_RED = 0,
        MSR_YELLOW = 1,
        MSR_BLINK = 2,

        SSR_RED = 0,
        SSR_TOP_YELLOW = 1,
        SSR_BOTTOM_YELLOW = 2,
        SSR_SIDE = 3,
        SSR_BLINK = 4,

        DSR_TOP_YELLOW = 0,
        DSR_GREEN = 1,
        DSR_BLINK = 2,

        BLINK_GREEN = 0,
        BLINK_YELLOW = 1
    };

    /// Главное сигнальное реле:
    /// включено, когда сигнал открыт на маршрут прямо
    Relay *main_signal_relay = new Relay(NUM_MSR_CONTACTS);

    /// Боковое сигнальное реле:
    /// включено, когда сигнал открыт на маршрут с отклонением по стрелкам
    Relay *side_signal_relay = new Relay(NUM_SSR_CONTACTS);

    /// Сигнальное реле сквозного пропуска:
    /// включено, когда следующий светофор открыт
    Relay *direct_signal_relay = new Relay(NUM_DSR_CONTACTS);

    /// Реле мигания верхнего желтого
    Relay *blink_relay = new Relay(NUM_BLINK_CONTACTS);

    /// Контакт мигания
    bool blink_contact = true;

    bool is_yellow_wire_ON = false;

    /// Таймер мигания верхнего желтого сигнала
    Timer *blink_timer = new Timer(0.75, false);

    void preStep(double t) override;

    /// Проверка состояния стрелок и занятости по маршруту до следующего светофора
    void check_route(bool& is_switches_side);

    /// Управление цепями питания реле
    void relay_control(bool is_switches_side);

    /// Управление миганием желтого (на предвходном)
    void yellow_blink_control();

    /// Управление состоянием линз
    void lens_control();

    /// Управление состоянием линий АЛСН
    void alsn_control();

private slots:

    void slotOnBlinkTimer();
};

#endif
