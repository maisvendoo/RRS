#ifndef     SIGNAL_H
#define     SIGNAL_H

#include    <device.h>
#include    <topology-export.h>
#include    <connector.h>
#include    <relay.h>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
enum
{
    NUM_LENS = 5,
    RED_LENS = 0,
    YELLOW_LENS = 1,
    GREEN_LENS = 2,
    BOTTOM_YELLOW_LENS = 3,
    CALL_LENS = 4
};

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
using lens_state_t = std::array<bool, NUM_LENS>;

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
enum
{
    NUM_ALSN_CTRL_LINES = 3,
    ALSN_RY_LINE = 0,
    ALSN_Y_LINE = 1,
    ALSN_G_LINE = 2
};

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
using alsn_state_t = std::array<bool, NUM_ALSN_CTRL_LINES>;

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

    void setLetter(const QString &letter)
    {
        this->letter = letter;
    }

    QString getLetter() const
    {
        return letter;
    }

    lens_state_t getAllLensState() const
    {
        return lens_state;
    }

    alsn_state_t getALSNstate() const
    {
        return alsn_state;
    }

    void setDirection(int signal_dir)
    {
        this->signal_dir = signal_dir;
    }

    int getDirection() const
    {
        return signal_dir;
    }

    void setConnector(Connector *conn)
    {
        this->conn = conn;
    }

    Connector *getConnector() const
    {
        return conn;
    }

    /// Установить занятость блок-участка, предшествующего данному сигналу
    void setBusy(bool is_busy)
    {
        this->is_busy = is_busy;
    }

    /// Задать имя модели сигнала
    void setSignalModel(const QString &signal_model)
    {
        this->signal_model = signal_model;
    }

    /// Вернуть тип сигнала (проходной/входной/выходной/маршрутный)
    QString getSignalType() const
    {
        return signal_model.right(4);
    }

    QByteArray serialize();

    void deserialize(QByteArray &data);

    QString getConnectorName() const
    {
        return conn_name;
    }

    double getLineVoltage() const
    {
        return U_line_prev;
    }

    double getVoltageDSR() const
    {
        return U_dsr;
    }

signals:

    /// Послать предыдущему световору напряжение линии
    void sendLineVoltage(double U_line);

    /// Послать серверу запрос на обновление данных
    void sendDataUpdate(QByteArray signal_data);

    /// Послать напряжение для бокового сигнального реле
    void sendSideVoltage(double U_side);

protected:    

    /// Состояние всех возможных линз
    lens_state_t lens_state;

    /// Предыдущее состояние ламп
    lens_state_t old_lens_state;

    /// Состояние линий управления трнасмитером АСЛН
    alsn_state_t alsn_state;

    /// Литер
    QString letter = "";

    /// Имя модели сигнала
    QString signal_model = "";

    /// Напряжение питания линейного реле
    double U_line = 0.0;

    double U_line_prev = 0.0;

    /// Напряжение питания бокового сигнального реле
    double U_side = 0.0;

    double U_dsr = 0.0;

    bool is_busy = false;

    int signal_dir = 0;

    /// Коннектор, с которым связан сигнал
    Connector *conn = Q_NULLPTR;

    /// Имя коннектора, с которым связан сигнал (для десериализации)
    QString conn_name = "";

    /// Реле управления линиями АЛСН
    enum
    {
        NUM_ALSN_RY_CONTACTS = 1,
        ALSN_RY = 0
    };

    Relay *alsn_RY_relay = new Relay(NUM_ALSN_RY_CONTACTS);

    enum
    {
        NUM_ALSN_Y_CONTACTS = 1,
        ALSN_Y = 0
    };

    Relay *alsn_Y_relay = new Relay(NUM_ALSN_Y_CONTACTS);

    enum
    {
        NUM_ALSN_G_CONTACTS = 1,
        ALSN_G = 0
    };

    Relay *alsn_G_relay = new Relay(NUM_ALSN_G_CONTACTS);

    void load_config(CfgReader &cfg) override;

public slots:

    /// Принять от следующего светофора напряжение на линии
    void slotRecvLineVoltage(double U_line);

    /// Принять от следующего светофора напряжение для БСР
    void slotRecvSideVoltage(double U_side);
};

#endif // SIGNAL_H
