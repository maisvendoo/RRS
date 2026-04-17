#ifndef     TRAIN_SIGNAL_H
#define     TRAIN_SIGNAL_H

#include    "rail-signal.h"

class Relay;
class Timer;

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class TOPOLOGY_EXPORT TrainSignal : public Signal
{
    Q_OBJECT

public:

    TrainSignal(QObject *parent = nullptr);

    virtual ~TrainSignal() override;

    /// Шаг симуляции
    virtual void step(double t, double dt) override;

    /// Код АЛСН
    alsn_state_t getALSNstate() const;

    /// Напряжение для линейного реле предыдущего светофора
    double getLineVoltage() const;

    /// Напряжение для бокового сигнального реле предыдущего светофора
    double getSideVoltage() const;

    void allowTransmitALSN(bool is_allow);

protected:

    /// Напряжение путевой батареи
    double U_bat = 12.0;

    /// Напряжение питания путевого реле
    double U_way = 0.0;

    /// Напряжение питания линейного реле
    double U_line = 0.0;

    /// Напряжение питания для линейного реле предыдущего светофора
    double U_line_prev = 0.0;

    /// Напряжение питания бокового сигнального реле
    double U_side = 0.0;

    /// Напряжение питания для бокового сигнального реле предыдущего светофора
    double U_side_prev = 0.0;

    /// Состояние линий управления трансмитером АСЛН
    alsn_state_t alsn_state;

    /// Признак разрешения работы путевого трансмитера
    /// (АЛСН не разрешается, если предыдущий светофор - закрытый станционный)
    bool is_alsn_allow = true;

    /// Признак работы путевого трансмитера
    bool is_asln_transmit = true;

    /// Реле управления линиями АЛСН
    enum
    {
        NUM_ALSN_RY_CONTACTS = 1,
        NUM_ALSN_Y_CONTACTS = 1,
        NUM_ALSN_G_CONTACTS = 1,

        ALSN_RY = 0,
        ALSN_Y = 0,
        ALSN_G = 0
    };

    /// Реле включения красного с жёлтым кода АЛСН
    Relay *alsn_RY_relay = nullptr;

    /// Реле включения жёлтого кода АЛСН
    Relay *alsn_Y_relay = nullptr;

    /// Реле включения зелёного кода АЛСН
    Relay *alsn_G_relay = nullptr;

    /// Таймер включения путевого трансмитера, если нет запрета
    Timer *alsn_allow_timer = nullptr;

private slots:

    /// Включить трансмиттер АЛСН по таймеру, если нет запрета
    void slotAllowTransmit();
};

#endif // TRAIN_SIGNAL_H
