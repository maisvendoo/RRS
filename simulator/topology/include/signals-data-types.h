#ifndef     SIGNALS_DATA_TYPES_H
#define     SIGNALS_DATA_TYPES_H

#include    <rail-signal.h>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
struct signals_data_t
{
    std::vector<Signal *> line_signals;

    QByteArray serialize()
    {
        QByteArray tmp_data;
        QBuffer buff(&tmp_data);
        buff.open(QIODevice::WriteOnly);
        QDataStream stream(&buff);

        for (auto line_signal : line_signals)
        {
            stream << line_signal->serialize();
        }

        return buff.data();
    }

    void deserialize(QByteArray &data)
    {
        QBuffer buff(&data);
        buff.open(QIODevice::ReadOnly);
        QDataStream stream(&buff);

        for (auto line_signal : line_signals)
        {
            QByteArray tmp_data;
            stream >> tmp_data;

            line_signal->deserialize(tmp_data);
        }
    }
};

#endif
