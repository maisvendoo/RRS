#ifndef     LINE_SIGNAL_H
#define     LINE_SIGNAL_H

#include    <rail-signal.h>
#include    <combine-relay.h>
#include    <timer.h>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class TOPOLOGY_EXPORT LineSignal : public Signal
{
public:

    LineSignal(QObject *parent = nullptr);

    ~LineSignal();

    void step(double t, double dt) override;

private:

    enum
    {
        WR_NUM = 1,
        WR_WAY_BUSY = 0
    };

    /// Путевое реле:
    /// включено, когда путь свободен (не зашунтирован колёсными парами)
    Relay *way_relay = new Relay(WR_NUM);

    enum
    {
        LR_NEUTRAL_NUM = 4,
        LR_NEUTRAL_LINE_PLUS = 0,
        LR_NEUTRAL_LINE_MINIS = 1,
        LR_NEUTRAL_ALLOW = 2,
        LR_NEUTRAL_PROHIBITING = 3,

        LR_PLUS_NUM = 1,
        LR_PLUS_GREEN = 0,

        LR_MINUS_NUM = 1,
        LR_MINUS_YELLOW = 0
    };

    /// Линейное реле (с полярным якорем):
    /// при питании положительным напряжением переключает на зелёный,
    /// отрицательным напряжением - жёлтый, без питания - красный
    CombineRelay *line_relay = new CombineRelay(LR_NEUTRAL_NUM,
                                                LR_PLUS_NUM,
                                                LR_MINUS_NUM);

    enum
    {
        NUM_SSR_CONTACTS = 2,
        SSR_GREEN = 0,
        SSR_YELLOW = 1
    };

    /// Боковое сигнальное реле (желтый мигающий, если следующий с отклонением по стрелкам)
    Relay *side_signal_relay = new Relay(NUM_SSR_CONTACTS);

    /// Таймер мигания желтого
    Timer *blink_timer = new Timer(0.75, false);

    /// Контакт мигания
    bool blink_contact = true;

    /// Напряжение путевой батареи
    double U_bat = 12.0;

    void preStep(state_vector_t &Y, double t) override;

    void ode_system(const state_vector_t &Y, state_vector_t &dYdt, double t) override;

    /// Проверка блок-участка до следующего светофора
    void check_route();

    /// Управление путевым и и линейным реле
    void relay_control();

    /// Управление миганием желтого (на предвходном)
    void yellow_blink_control();

    /// Управление состоянием линз
    void lens_state_control();

    /// Управление состоянием линий АЛСН
    void alsn_control();

private slots:

    void slotBlinkTimer();
};

#endif
