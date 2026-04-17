#ifndef     SIGNAL_H
#define     SIGNAL_H

#include    <QObject>

#include    <vec3.h>
#include    "topology-export.h"
#include    "topology-defines.h"
#include    "signal-types.h"

class CfgReader;
class Switch;

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

    void setConnector(Switch* conn);
    Switch* getConnector() const;

    void setDirection(dir_t dir);
    dir_t getDirection() const;

    void setLetter(const QString& letter);
    QString getLetter() const;

    /// Задать имя модели сигнала
    void setSignalModel(const QString& signal_model);
    QString getSignalModel() const;

    /// Вернуть тип сигнала (проходной/входной/выходной/маршрутный)
    QString getSignalType() const;

    lens_state_t getAllLensState() const;

    void setRelPosition(const dvec3& rel_pos);
    void setRelRotation(const dvec3& rel_rot);

    QByteArray serialize() const;

    void deserialize(QByteArray& data);

    bool calcPosition();

    const dvec3& getPos() const;
    const dvec3& getRight() const;
    const dvec3& getOrth() const;
    const dvec3& getUp() const;

    const dvec3& getRelPos() const;
    const dvec3& getRelRot() const;

signals:

    /// Послать серверу запрос на обновление данных
    void sendDataUpdate(QByteArray signal_data);

private:

    /// Предыдущее состояние огней светофора
    lens_state_t old_lens_state;

protected:

    dir_t signal_dir = FWD;

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
    Switch* conn = nullptr;

    virtual void preStep(double t);

    virtual void load_config(CfgReader& cfg);
};

#endif // SIGNAL_H
