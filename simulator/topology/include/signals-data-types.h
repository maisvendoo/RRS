#ifndef     SIGNALS_DATA_TYPES_H
#define     SIGNALS_DATA_TYPES_H

#include    <line-signal.h>

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

        stream << line_signals.size();

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

        size_t data_size = 0;
        stream >> data_size;

        line_signals.clear();

        for (size_t i = 0; i < data_size; ++i)
        {
            QByteArray tmp_data;
            stream >> tmp_data;

            LineSignal *line_signal = new LineSignal;
            line_signal->deserialize(tmp_data);

            line_signals.push_back(line_signal);
        }
    }
};

#endif
