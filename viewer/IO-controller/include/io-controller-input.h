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

    /// Код назначенной клавиши
    uint16_t keyCode = 0;
    /// Имя объекта в 3D-модели
    QString contolledObjectName = "";

    /// Отображаемое имя органа управления (подсказка по Alt+наведению)
    QString name = "";
    /// Тип органа (Toggle, Button, Crane395, Crane254, KM, Revers,
    /// Lock367, Lever, Gauge - семантика задаётся аддоном)
    QString type = "";
    /// Сигнал ПЕ с текущим состоянием органа (для тултипа и
    /// вычисления целевого значения при клике мышью)
    int signal_id = -1;
    /// Второй сигнал (например, реверс: вставлена ли рукоятка)
    int signal_id2 = -1;
    /// Режим расшифровки состояния (norm, centered, kme)
    QString state_mode = "";
    /// Имена состояний через ';'
    QString state_names = "";

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
