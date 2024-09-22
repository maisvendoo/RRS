#ifndef     SIGNALS_DATA_TYPES_H
#define     SIGNALS_DATA_TYPES_H

#include    <line-signal.h>
#include    <enter-signal.h>
#include    <exit-signal.h>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
struct signals_data_t
{
    std::vector<Signal *> line_signals;

    std::vector<Signal *> enter_signals;

    std::vector<Signal *> exit_signals;


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

        stream << enter_signals.size();

        for (auto enter_signal: enter_signals)
        {
            stream << enter_signal->serialize();
        }

        stream << exit_signals.size();

        for (auto signal : exit_signals)
        {
            stream << signal->serialize();
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

        size_t enter_signals_size = 0;
        stream >> enter_signals_size;

        enter_signals.clear();

        for (size_t i = 0; i < enter_signals_size; ++i)
        {
            QByteArray tmp_data;
            stream >> tmp_data;

            EnterSignal *enter_signal = new EnterSignal;
            enter_signal->deserialize(tmp_data);

            enter_signals.push_back(enter_signal);
        }

        size_t exit_signals_size = 0;
        stream >> exit_signals_size;

        exit_signals.clear();

        for (size_t i = 0; i < exit_signals_size; ++i)
        {
            QByteArray tmp_data;
            stream >> tmp_data;

            ExitSignal *signal = new ExitSignal();
            signal->deserialize(tmp_data);

            exit_signals.push_back(signal);
        }
    }
};

#endif
