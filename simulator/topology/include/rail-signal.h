#ifndef     SIGNAL_H
#define     SIGNAL_H

#include    <QObject>

#include    <vec3.h>
#include    "topology-export.h"
#include    "signal-types.h"

class CfgReader;
class Connector;

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class TOPOLOGY_EXPORT Signal : public QObject
{
    Q_OBJECT

public:

    Signal(QObject *parent = nullptr);

    virtual ~Signal();

    /// Шаг симуляции
    virtual void step(double t, double dt);

    /// Чтение конфиг-файла filename из dir_path, либо по умолчанию из cfg/devices
    virtual void read_config(const QString& filename, const QString& dir_path = "");

    QString getConnectorName() const;

    void setConnector(Connector* conn);

    Connector* getConnector() const;

    void setDirection(int signal_dir)
    {
        this->signal_dir = signal_dir;
    }

    int getDirection() const
    {
        return signal_dir;
    }

    void setLetter(const QString& letter)
    {
        this->letter = letter;
    }

    QString getLetter() const
    {
        return letter;
    }

    /// Задать имя модели сигнала
    void setSignalModel(const QString& signal_model)
    {
        this->signal_model = signal_model;
    }

    QString getSignalModel() const
    {
        return this->signal_model;
    }

    /// Вернуть тип сигнала (проходной/входной/выходной/маршрутный)
    QString getSignalType() const
    {
        return signal_model.right(4);
    }

    lens_state_t getAllLensState() const
    {
        return lens_state;
    }

    void setRelPosition(dvec3 rel_pos)
    {
        this->rel_pos = rel_pos;
    }

    void setRelRotation(dvec3 rel_rot)
    {
        this->rel_rot = rel_rot;
    }

    QByteArray serialize();

    void deserialize(QByteArray& data);

    bool calcPosition();

    dvec3 getPos() const
    {
        return pos;
    }

    dvec3 getRight() const
    {
        return right;
    }

    dvec3 getOrth() const
    {
        return orth;
    }

    dvec3 getUp() const
    {
        return up;
    }

signals:

    /// Послать серверу запрос на обновление данных
    void sendDataUpdate(QByteArray signal_data);

private:

    /// Предыдущее состояние огней светофора
    lens_state_t old_lens_state;

protected:

    int signal_dir = 0;

    /// Состояние всех возможных огней светофора
    lens_state_t lens_state;

    /// Имя коннектора, с которым связан сигнал (для десериализации)
    QString conn_name = "";

    /// Литер
    QString letter = "";

    /// Имя модели сигнала
    QString signal_model = "";

    /// Вектор смещения относительно коннектора и трека
    dvec3 rel_pos = {0.0, 0.0, 0.0};

    /// Вектор поворота относительно конектора и трека
    dvec3 rel_rot = {0.0, 0.0, 0.0};

    /// Орт вдоль оси X собственной системы координат светофора
    dvec3 right = {1.0, 0.0, 0.0};

    /// Орт вдоль оси Y собственной системы координат светофора
    dvec3 orth = {0.0, 1.0, 0.0};

    /// Орт вдоль оси Z собственной системы координат светофора
    dvec3 up = {0.0, 0.0, 1.0};

    /// Абсолютное положение сигнала
    dvec3 pos = {0.0, 0.0, 0.0};

    /// Коннектор, с которым связан сигнал
    Connector* conn = nullptr;

    virtual void preStep(double t);

    virtual void load_config(CfgReader& cfg);
};

#endif // SIGNAL_H
