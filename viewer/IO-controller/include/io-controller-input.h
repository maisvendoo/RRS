#ifndef     IO_CONTROLLER_INPUT_H
#define     IO_CONTROLLER_INPUT_H

#include    <QString>
#include    <QByteArray>
#include    <QDataStream>
#include    <QIODevice>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
struct io_control_input_t
{
    /// Индекс управляемой ПЕ
    int controlled_vehicle_idx = 0;
    /// Индекс активной кабины управляемой ПЕ
    int cabine_idx = 0;
    /// Идентификатор сигнала управления в массиве сигналов ПЕ
    uint16_t id = 0;
    /// Значение управляющего сигнала
    float value = 0.0f;

    /// Имя назначенной клавиши
    QString keyName = "";
    /// Имя объекта в 3D-модели
    QString contolledObjectName = "";

    io_control_input_t()
    {

    }

    QByteArray serialize() const
    {
        QByteArray data;
        QDataStream stream(&data, QIODevice::WriteOnly);

        // Серверу передаем только эти значения
        stream << controlled_vehicle_idx;
        stream << cabine_idx;
        stream << id;
        stream << value;

        return data;
    }

    void deserialize(QByteArray &data)
    {
        QDataStream stream(&data, QIODevice::ReadOnly);

        stream >> controlled_vehicle_idx;
        stream >> cabine_idx;
        stream >> id;
        stream >> value;
    }

    bool toBool() const
    {
        return static_cast<bool>(value);
    }
};

#endif
