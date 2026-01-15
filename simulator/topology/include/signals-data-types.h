#ifndef     SIGNALS_DATA_TYPES_H
#define     SIGNALS_DATA_TYPES_H

#include    <QBuffer>
#include    "line-signal.h"
#include    "enter-signal.h"
#include    "route-signal.h"
#include    "exit-signal.h"
#include    "shunting-signal.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
struct signals_data_t
{
    std::vector<Signal *> line_signals;

    std::vector<Signal *> enter_signals;

    std::vector<Signal *> route_signals;

    std::vector<Signal *> exit_signals;

    std::vector<Signal *> shunt_signals;


    QByteArray serialize()
    {
        QByteArray tmp_data;
        QBuffer buff(&tmp_data);
        buff.open(QIODevice::WriteOnly);
        QDataStream stream(&buff);

        stream << static_cast<uint32_t>(line_signals.size());

        for (auto line_signal : line_signals)
        {
            stream << line_signal->serialize();
        }

        stream << static_cast<uint32_t>(enter_signals.size());

        for (auto entr_signal: enter_signals)
        {
            stream << entr_signal->serialize();
        }

        stream << static_cast<uint32_t>(route_signals.size());

        for (auto rout_signal : route_signals)
        {
            stream << rout_signal->serialize();
        }

        stream << static_cast<uint32_t>(exit_signals.size());

        for (auto exit_signal : exit_signals)
        {
            stream << exit_signal->serialize();
        }

        stream << static_cast<uint32_t>(shunt_signals.size());

        for (auto shnt_signal : shunt_signals)
        {
            stream << shnt_signal->serialize();
        }

        return buff.data();
    }

    void deserialize(QByteArray &data)
    {
        QBuffer buff(&data);
        buff.open(QIODevice::ReadOnly);
        QDataStream stream(&buff);

        uint32_t size = 0;
        stream >> size;
        line_signals.clear();

        for (uint32_t i = 0; i < size; ++i)
        {
            QByteArray tmp_data;
            stream >> tmp_data;

            LineSignal *line_signal = new LineSignal;
            line_signal->deserialize(tmp_data);

            line_signals.push_back(line_signal);
        }

        size = 0;
        stream >> size;
        enter_signals.clear();

        for (uint32_t i = 0; i < size; ++i)
        {
            QByteArray tmp_data;
            stream >> tmp_data;

            EnterSignal *enter_signal = new EnterSignal;
            enter_signal->deserialize(tmp_data);

            enter_signals.push_back(enter_signal);
        }

        size = 0;
        stream >> size;
        route_signals.clear();

        for (uint32_t i = 0; i < size; ++i)
        {
            QByteArray tmp_data;
            stream >> tmp_data;

            RouteSignal *rout_signal = new RouteSignal;
            rout_signal->deserialize(tmp_data);

            route_signals.push_back(rout_signal);
        }

        size = 0;
        stream >> size;
        exit_signals.clear();

        for (uint32_t i = 0; i < size; ++i)
        {
            QByteArray tmp_data;
            stream >> tmp_data;

            ExitSignal *signal = new ExitSignal();
            signal->deserialize(tmp_data);

            exit_signals.push_back(signal);
        }

        size = 0;
        stream >> size;
        shunt_signals.clear();

        for (uint32_t i = 0; i < size; ++i)
        {
            QByteArray tmp_data;
            stream >> tmp_data;

            ShuntingSignal *shnt_signal = new ShuntingSignal;
            shnt_signal->deserialize(tmp_data);

            shunt_signals.push_back(shnt_signal);
        }

    }
};

#endif
