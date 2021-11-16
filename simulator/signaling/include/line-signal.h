#ifndef     LINE_SIGNAL_H
#define     LINE_SIGNAL_H

#include    "abstract-signal.h"
#include    "logic-relay.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
enum
{
    LINE_RED = 0,
    LINE_GREEN = 1,
    LINE_YELLOW = 2
};

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class LineSignal : public Signal
{
public:

    LineSignal(QObject *parent = Q_NULLPTR);

    ~LineSignal();

    void close(bool is_closed) override;

    void step(double t, double dt) override;

private:

    /// Реле, питаемое от рельсовой цепи
    LogicRelay *busy_relay;

    /// Реле переключения на желтый
    LogicRelay *yellow_relay;

    void preStep(state_vector_t &Y, double t) override;

    void load_config(CfgReader &cfg) override;

private slots:

    void slotChangeState() override;
};

#endif // LINE_SIGNAL_H
