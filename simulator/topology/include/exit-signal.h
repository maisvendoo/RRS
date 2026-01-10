#ifndef     EXIT_SIGNAL_H
#define     EXIT_SIGNAL_H

#include    <rail-signal.h>
#include    <combine-relay.h>
#include    <timer.h>
#include    "trigger.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class TOPOLOGY_EXPORT ExitSignal : public Signal
{
public:

    ExitSignal(QObject *parent = nullptr);

    ~ExitSignal();

    void step(double t, double dt) override;

public slots:

    void slotPressOpen();

    void slotPressClose();

private:

    enum
    {
        NUM_RCR_CONTACTS = 2,
        NUM_SR_CONTACTS = 5,
        NUM_DLR_CONTACTS = 1,
        NUM_SSR_CONTACTS = 2,

        RCR_SR_CTRL = 0,
        RCR_SRS_CTRL = 1,

        SR_SELF = 0,
        SR_DLR_CTRL = 1,
        SR_SRS_CTRL = 2,
        SR_PLUS = 3,
        SR_MINUS = 4,

        DRL_CTRL = 0,

        SSR_GREEN = 0,
        SSR_YELLOW = 1
    };

    enum
    {
        NUM_SRS_NEUTRAL_CONTACTS = 3,
        NUM_SRS_PLUS_CONTACTS = 1,
        NUM_SRS_MINUS_CONTACTS = 1,

        SRS_N_RED = 0,
        SRS_N_ALLOW = 1,

        SRS_PLUS_GREEN = 0,

        SRS_MINUS_YELLOW = 0
    };

    /// Контрольное маршрутное реле:
    /// включено, когда до следующего светофора свободно и стрелки по маршруту
    Relay *route_control_relay = new Relay(NUM_RCR_CONTACTS);

    /// Сигнальное реле:
    /// управляется кнопками открыть/закрыть сигнал (если маршрут возможен)
    Relay *signal_relay = new Relay(NUM_SR_CONTACTS);

    /// Реле замыкания маршрута отправления:
    /// повторяет сигнальное реле, в будущем должно блокировать стрелки от перевода
    Relay *departure_lock_relay = new Relay(NUM_DLR_CONTACTS);

    /// Сигнальное реле светофора (с полярным якорем):
    /// при питании положительным напряжением переключает на зелёный,
    /// отрицательным напряжением - жёлтый, без питания - красный
    CombineRelay *semaphore_signal_relay = new CombineRelay(NUM_SRS_NEUTRAL_CONTACTS,
                                                            NUM_SRS_PLUS_CONTACTS,
                                                            NUM_SRS_MINUS_CONTACTS);

    /// Боковое сигнальное реле (желтый мигающий, если следующий с отклонением по стрелкам)
    Relay *side_signal_relay = new Relay(NUM_SSR_CONTACTS);
/*
    enum
    {
        NUM_YR_CONTACTS = 3,
        YR_SR_CTRL = 0,
        YR_SRS_PLUS = 1,
        YR_ALSN_CTRL = 2
    };

    /// Реле контроля первого участка удаления
    Relay *yellow_relay = new Relay(NUM_YR_CONTACTS);

    enum
    {
        NUM_GR_CONTACTS = 2,
        GR_SRS_MINUS = 0,
        GR_SRS_PLUS = 1
    };

    /// Реле контроля второго участка удаления
    Relay *green_relay = new Relay(NUM_GR_CONTACTS);

    enum
    {
        NUM_FWD_WR_CONTACTS = 1,
        FWD_BUSY = 0
    };

    /// Путевое реле стрелочного участка за светофором
    Relay *fwd_way_relay = new Relay(NUM_FWD_WR_CONTACTS);

    enum
    {
        NUM_AR_CONTACTS = 1,
        AR_OPEN = 0
    };

    /// Указательное реле, для связи с предыдущим входным светофором
    Relay *allow_relay = new Relay(NUM_AR_CONTACTS);

    /// Линейное реле, для связи со следующим светофором
    enum
    {
        NUM_LINE_NEUTRAL_CONTACTS = 1,
        NUM_LINE_PLUS_CONTACTS = 1,
        NUM_LINE_MINUS_CONTACTS = 0,

        LINE_N_YELLOW = 0,
        LINE_PLUS_GREEN = 0
    };

    CombineRelay *line_relay = new CombineRelay(NUM_LINE_NEUTRAL_CONTACTS,
                                                NUM_LINE_PLUS_CONTACTS,
                                                NUM_LINE_MINUS_CONTACTS);

*/
    double U_bat = 12.0;

    /// Признак нажатия кнопки открытия
    bool is_open_button_pressed = false;

    /// Признак НЕнажатия кнопки закрытия (нормально замкнутая)
    bool is_close_button_unpressed = true;

    /// Контакт мигания
    bool blink_contact = true;

    /// Таймер выдержкм времени удержания кнопки открыть
    Timer *open_timer = new Timer(1.0, false);

    /// Таймер выдержки времени удержания кнопки закрыть
    Timer *close_timer = new Timer(1.0, false);

    /// Таймер мигания желтого
    Timer *blink_timer = new Timer(0.75, false);

    Signal *next_signal = nullptr;

    void preStep(state_vector_t &Y, double t) override;

    void ode_system(const state_vector_t &Y,
                    state_vector_t &dYdt,
                    double t) override;

    /// Проверка состояния стрелок и занятости по маршруту до следующего светофора
    void check_route();

    /// Управление цепями питания реле
    void relay_control();

    /// Управление миганием желтого (на предвходном)
    void yellow_blink_control();

    /// Управление состоянием линз
    void lens_control();

    /// Управление состоянием линий АЛСН
    void alsn_control();
/*
    void lens_control();

    void fwd_way_busy_control();

    void removal_area_control();

    void route_control(Signal **next_signal);

    void relay_control();

    void alsn_control();

    /// Проверка занятости сигнала на участке от данного коннектора
    /// до следующего попутного сигнала
    Connector *check_path_free(Connector *cur_conn, bool &is_free);
*/
private slots:

    void slotOpenTimer();

    void slotCloseTimer();

    void slotBlinkTimer();
};

#endif
