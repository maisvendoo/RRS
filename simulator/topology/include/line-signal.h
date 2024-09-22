#ifndef     LINE_SIGNAL_H
#define     LINE_SIGNAL_H

#include    <rail-signal.h>
#include    <combine-releay.h>
#include    <timer.h>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class TOPOLOGY_EXPORT LineSignal : public Signal
{
public:

    LineSignal(QObject *parent = Q_NULLPTR);

    ~LineSignal();

    void step(double t, double dt) override;

private:

    enum
    {
        WR_NEUTRAL_NUM = 1,
        WR_NEUTRAL_WAY_BUSY = 0
    };

    /// Путевое реле - используем только нейтральный якорь
    CombineRelay *way_relay = new CombineRelay(WR_NEUTRAL_NUM, 0, 0);

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

    /// Линейное реле
    CombineRelay *line_relay = new CombineRelay(LR_NEUTRAL_NUM,
                                                LR_PLUS_NUM,
                                                LR_MINUS_NUM);

    enum
    {
        NUM_SSR_CONTACTS = 2,
        SSR_GREEN = 0,
        SSR_YELLOW = 1
    };

    /// Боковое сигнальное реле (желтый мигающий для предвходного)
    Relay *side_signal_relay = new Relay(NUM_SSR_CONTACTS);

    /// Таймер мигания желтого
    Timer *blink_timer = new Timer(1.0, false);

    /// Контакт мигания
    bool blink_contact = true;

    /// Напряжение путевой батареи
    double U_bat = 12.0;

    /// Напряжение, передаваемое на линию предыдущего светофора
    double U_line_prev = 0.0;

    void preStep(state_vector_t &Y, double t) override;

    void ode_system(const state_vector_t &Y, state_vector_t &dYdt, double t) override;

private slots:

    void slotBlinkTimer();
};

#endif
