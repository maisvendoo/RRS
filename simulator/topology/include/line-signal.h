#ifndef     LINE_SIGNAL_H
#define     LINE_SIGNAL_H

#include    "train-signal.h"

class CombineRelay;
class Relay;
class Timer;

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class TOPOLOGY_EXPORT LineSignal : public TrainSignal
{
public:

    LineSignal(QObject *parent = nullptr);

    ~LineSignal();

    void step(double t, double dt) override;

private:

    enum
    {
        WR_WAY_BUSY = 0,
        WR_NUM,
    };

    /// Путевое реле:
    /// включено, когда путь свободен (не зашунтирован колёсными парами)
    Relay *way_relay = nullptr;

    enum
    {
        LR_NEUTRAL_LINE_PLUS = 0,
        LR_NEUTRAL_LINE_MINIS,
        LR_NEUTRAL_ALLOW,
        LR_NEUTRAL_PROHIBITING,
        LR_NEUTRAL_NUM,

        LR_PLUS_GREEN = 0,
        LR_PLUS_NUM,

        LR_MINUS_YELLOW = 0,
        LR_MINUS_NUM,
    };

    /// Линейное реле (с полярным якорем):
    /// при питании положительным напряжением переключает на зелёный,
    /// отрицательным напряжением - жёлтый, без питания - красный
    CombineRelay *line_relay = nullptr;

    enum
    {
        SSR_GREEN = 0,
        SSR_YELLOW,
        NUM_SSR_CONTACTS,
    };

    /// Боковое сигнальное реле (желтый мигающий, если следующий с отклонением по стрелкам)
    Relay *side_signal_relay = nullptr;

    /// Таймер мигания желтого
    Timer *blink_timer = nullptr;

    /// Контакт мигания
    bool blink_contact = true;

    void preStep(double t) override;

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
