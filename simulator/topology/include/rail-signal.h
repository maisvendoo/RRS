#ifndef     SIGNAL_H
#define     SIGNAL_H

#include    <device.h>
#include    <topology-export.h>
#include    <connector.h>

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

    /// Литер
    QString letter = "";

    /// Имя модели сигнала
    QString signal_model = "";

    /// Напряжение питания линейного реле
    double U_line = 0.0;

    /// Напряжение питания бокового сигнального реле
    double U_side = 0.0;

    bool is_busy = false;

    int signal_dir = 0;

    /// Коннектор, с которым связан сигнал
    Connector *conn = Q_NULLPTR;

    /// Имя коннектора, с которым связан сигнал (для десериализации)
    QString conn_name = "";

    void load_config(CfgReader &cfg) override;

public slots:

    /// Принять от следующего светофора напряжение на линии
    void slotRecvLineVoltage(double U_line);

    /// Принять от следующего светофора напряжение для БСР
    void slotRecvSideVoltage(double U_side);
};

#endif // SIGNAL_H
