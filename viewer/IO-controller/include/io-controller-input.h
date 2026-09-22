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
    int controlled_vehicle_idx = -1;
    /// Индекс активной кабины управляемой ПЕ
    int cabine_idx = -1;
    /// Идентификатор сигнала управления в массиве сигналов ПЕ
    uint16_t id = 0;
    /// Значение управляющего сигнала
    float value = 0.0f;
    /// Дополнительное значение
    float value2 = 0.0f;

    /// Код назначенной клавиши
    uint16_t keyCode = 0;
    /// Имя модификатора включения
    QString keyModOnName = "";
    /// Имя модификатора выключения
    QString keyModOffName = "";
    /// Имя объекта в 3D-модели
    QString contolledObjectName = "";
    /// Имя контрола
    QString name = "";
    /// Тип контрола
    QString type = "";
    /// Описание контрола
    QString description = "";
    /// Использование контрола
    QString usage = "";
    /// Описание горячей клавиши
    QString hot_keys = "";

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

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
enum
{
    IO_CTRL_LEFT_MOUSE_BUTTON = 1,
    IO_CTRL_MIDDLE_MOUSE_BUTTON = 2,
    IO_CTRL_RIGHT_MOUSE_BUTTON = 3,
};

#endif
