#ifndef     SLAVE_H
#define     SLAVE_H

#include    <QObject>
#include    <QMap>

#include    "CfgReader.h"
#include    "slave-data.h"

typedef  QMap<quint16, slave_data_t> data_map_t;

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class Slave : public QObject
{
public:

    Slave(QObject *parent = Q_NULLPTR);

    ~Slave();

    /// Идентификатор устройства в сети (от 1 до 31)
    quint16     id;

    /// Флаг необходимости конфигурирования
    bool        is_config_required;

    /// Маска конфигурирования входов/выходов
    quint16     cells_mask;


    /// Описание устройства
    QString     description;

private:

    /// Флаг наличия связи с устройством
    bool        is_connected;

    /// Счетчик ошибок связи
    quint32     errors;

    /// Максимальное допускаемое до отключения число ошибок связи
    quint32     maxErrors;

public:

    /// Дискретные входы
    data_map_t      discrete_input;

    /// Дискретные выходы
    data_map_t      coil;

    /// Регистры ввода
    data_map_t      input_register;

    /// Регистры вывода
    data_map_t      holding_register;

    bool isConfigRequired() const;

    bool isConnected() const;

    bool load_config(QString cfg_path);

private:

    /// Загрузка конфигурации структуры данных определенного типа
    void load_data_structure(QString name, CfgReader &cfg, data_map_t &data);
};


#endif // SLAVE_H
